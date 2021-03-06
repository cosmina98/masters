#ifndef HBM_EDUK_HPP
#define HBM_EDUK_HPP

#include <algorithm>
#include <memory> // for unique_ptr
#include <forward_list>
#include <vector>

#include "ukp_common.hpp"
#include "wrapper.hpp"
#include "type_name.hpp"

// The original version of the algorithm relies on the garbage collection of
// functional languages, so nodes of lazy lists that are unreachable by the
// code are silently and automatically dealocated. In the imperative version
// the nodes don't become unreachable, but they are kept even if they aren't
// needed anymore. This definition controls the maximum of unneeded nodes a
// lazy list can have. If the list (a vector in truth) has more unneeded
// nodes/positions than HBM_MAX_MEMORY_WASTED_BY_S we copy the nodes that are
// yet needed into another vector and swap both vectors (i.e. garbage
// collection by hand).
#ifndef HBM_MAX_MEMORY_WASTED_BY_S
  #define HBM_MAX_MEMORY_WASTED_BY_S 10
#endif

namespace hbm {
  template <typename W, typename P, typename I>
  struct eduk_extra_info_t : extra_info_t {
    eduk_extra_info_t(void) { }

    virtual std::string gen_info(void) {
      return std::string("algorithm_name: eduk\n")
           + "git_head_at_compilation: " + HBM_GIT_HEAD_AT_COMPILATION + "\n"
           + "type_W: " + hbm::type_name<W>::get() + "\n"
           + "type_P: " + hbm::type_name<P>::get() + "\n"
           + "type_I: " + hbm::type_name<I>::get() + "\n";
    }
  };

  /// Implementation details. Do not depend.
  ///
  /// This is an adaptation of the algorithm described at "Sparse knapsack
  /// algo-tech-cuit and its synthesis". The original algorithm is described
  /// using a functional programming language, and makes use of lazy evaluation.
  /// A most faithful adaptation is provided at codes/hs/ukp.hs and is written
  /// in haskell.
  ///
  /// The basic ideia is to implement the lazy list concept as a class, and
  /// create a specific subclass for every lazy function that was applied over
  /// lazy lists in the original algorithm. A lazy list is basically a list
  /// that only really compute its elements when they are requested by a
  /// non-lazy function. So if you applies a lot of lazy functions over a lazy
  /// list those data transformations don't really happen. The computation only
  /// occurs when you ask with a non-lazy functions for the real list values.
  /// You ask for the first node and then it will compute all that is needed to
  /// know the value of that node, and the remaining nodes will remain
  /// uncomputed. The consume function is our non-lazy function. We apply a
  /// lot of lazy functions over a lazy list (describing a complex data flow
  /// that is hard to denote on imperative languages), and then we call consume
  /// to put this flow to work (it will ask the nodes one by one until all the
  /// list is processed).
  ///
  /// We need to do the things this way because the main data structure of this
  /// algorithm is a self-referential list (in truth, a bunch of
  /// self-referential lists). In other words, the list uses nodes from many
  /// auxiliar lists to compute new nodes to the main list, and every node of
  /// the main list spawn a new auxiliar list to be used in this data flow.
  /// Those auxiliar lists have a simple formula for generation, but depending
  /// on its values we don't need to evaluate all of its values (it would be
  /// even more memory consuming) so we only compute the nodes from those lists
  /// when the main list really need them.
  namespace hbm_eduk_impl {
    using namespace std;

    template<typename W, typename P>
    bool weight_order(const item_t<W, P>& i, const item_t<W, P>& j) {
      return i.w < j.w;
    }

    template<typename W, typename P>
    void sort_by_weigth(vector< item_t<W, P> > &items) {
      sort(items.begin(), items.end(), weight_order<W, P>);

      return;
    }

    template<typename W, typename P>
    struct LazyList {
      /// The main method. Computes only the next node of the list and returns
      /// it. Each lazy operation needs to redefine it.
      virtual bool get_next(item_t<W, P>& x) = 0;
    };

    /// Our non-lazy (i.e. strict) function. It forces a lazy list to compute
    /// all its nodes.
    template<typename W, typename P>
    void consume(LazyList<W, P> * const l, vector< item_t<W, P> > &res) {
      item_t<W, P> i;
      bool has_head = l->get_next(i);
      while (has_head) {
        res.push_back(i);
        has_head = l->get_next(i);
      }
    }

    /// A simple wrapper class, makes a LazyList from a vector.
    template<typename W, typename P>
    struct Lazyfy : LazyList<W, P> {
      typename vector< item_t<W, P> >::const_iterator begin;
      typename vector< item_t<W, P> >::const_iterator end;

      Lazyfy(void) {
      }

      /// Makes reference to the v argument, v can't be freed before this
      /// struct. 
      Lazyfy(const vector< item_t<W, P> > &v) {
        begin = v.cbegin();
        end = v.cend();
      }

      bool get_next(item_t<W, P>& i) {
        if (begin != end) {
          i = *begin;
          ++begin;
          return true;
        } else {
          return false;
        }
      }
    };

    /// Add g to every item of l (by adding items we mean: create an item where
    /// the profit is the sum of the profit of the two items, and the weight is
    /// analogue), and returns it if its weight is smaller than c.
    template<typename W, typename P>
    struct AddTest : LazyList<W, P> {
      item_t<W, P> g;
      LazyList<W, P> * l;
      W c;

      AddTest(void) {
        g = {0, 0};

        l = nullptr;
        c = 0;
      }

      AddTest(item_t<W, P> g, LazyList<W, P> * l, W c) : g(g), l(l), c(c) { }

      bool get_next(item_t<W, P>& i) {
        bool has_next = l->get_next(i);
        i.w += g.w;
        i.p += g.p;
        return has_next && i.w <= c;
      }
    };

    /// Gets the first item in the list l with a profit BIGGER than limit.
    template<typename W, typename P>
    struct Filter : LazyList<W, P> {
      LazyList<W, P> * l;
      P limit;

      Filter(void) {
        limit = 0;
        l = nullptr;
      }

      Filter(LazyList<W, P> * l, P limit) : l(l), limit(limit) { }

      bool get_next(item_t<W, P>& i) {
        bool has_next = l->get_next(i);
        while (has_next && i.p <= limit) {
          has_next = l->get_next(i);
        }
        if (has_next) {
          limit = i.p;
        }

        return has_next;
      }
    };

    /// Merge two LazyLists, as in mergesort, always choose the smaller between
    /// the front of the two list to go next (in this case the smaller will be
    /// the one of lowest weight).
    template<typename W, typename P>
    struct Merge : LazyList<W, P> {
      LazyList<W, P> * l1, * l2;
      item_t<W, P> h1, h2;
      bool has_h1, has_h2, has_old_h1, has_old_h2;

      Merge(void) {
        has_h1 = has_h2 = has_old_h1 = has_old_h2 = false;
        l1 = l2 = nullptr;
        h1 = h2 = {0, 0};
      }

      Merge(LazyList<W, P> * l1, LazyList<W, P> * l2) : l1(l1), l2(l2) {
        has_old_h1 = has_old_h2 = false;
      }

      bool get_next(item_t<W, P>& i) {
        if (!has_old_h1) has_h1 = l1->get_next(h1);
        if (!has_old_h2) has_h2 = l2->get_next(h2);

        if ((has_h1 || has_old_h1) && (has_h2 || has_old_h2)) {
          if (h1.w < h2.w) {
            i = h1;
            has_old_h1 = false;
            has_old_h2 = true;
          } else if (h1.w > h2.w) {
            i = h2;
            has_old_h1 = true;
            has_old_h2 = false;
          } else {
            i.w = h1.w;
            i.p = max(h1.p, h2.p);
            has_old_h1 = false;
            has_old_h2 = false;
          }
        } else if (has_h1 || has_old_h1) {
          i = h1;
          has_old_h1 = false;
        } else if (has_h2 || has_old_h2) {
          i = h2;
          has_old_h2 = false;
        } else { /* The only possibility is: all the four flags are false */
          return false;
        }

        return true;
      }
    };

    /// The first time get_next is called it returns the head parameter
    /// any other call will simply return l->get_next.
    template<typename W, typename P>
    struct AddHead : LazyList<W, P> {
      item_t<W, P> original_head;
      LazyList<W, P> * l;

      bool has_original_head;

      AddHead(void) {
        l = nullptr;
        has_original_head = false;
        original_head = {0, 0};
      }

      AddHead(const item_t<W, P> &head, LazyList<W, P> * l) : original_head(head), l(l) {
        has_original_head = true;
      }

      bool get_next(item_t<W, P>& i) {
        if (has_original_head) {
          i = original_head;
          has_original_head = false;
          return true;
        } else {
          return l->get_next(i);
        }
      }
    };

    /// This empty reference is needed to allow the class to be
    /// self-referential.
    template<typename W, typename P, typename I = size_t>
    struct S;
    template<typename W, typename P, typename I>
    struct S {
      unique_ptr< S<W, P, I> > s_pred_k;
      Lazyfy<W, P>  l_items;
      AddHead<W, P> addhead;
      AddTest<W, P> addtest;
      Merge<W, P>   merge;
      Filter<W, P>  filter;

      bool empty;

      vector< item_t<W, P> > computed;

      struct iterator : LazyList<W, P> {
        S<W, P, I> * s;
        size_t ix;

        iterator(S<W, P, I> * s) : s(s) {
          ix = 0;
        }

        bool get_next(item_t<W, P> &i) {
          // Using the handmade garbage_collect was not paying off
          //s->garbage_collect();
          if (ix < s->computed.size() || s->has_next()) {
            i = s->computed[ix];
            ++ix;
            return true;
          } else {
            return false;
          }
        }
      };

      /* Only to avoid allocating dynamic memory by hand
       * HAS TO BE A LIST, LOST TWO HOURS DEBUGGING BECAUSE
       * USING VECTOR IS A BAD IDEA (WHEN SIZE BECOMES BIGGER THAN CAPACITY
       * THE INTERNAL DATA IS REALLOCATED AND THE OLD POINTERS TO INTERNAL
       * OBJECTS LOSE THEIR MEANING)
       * */
      forward_list< S<W, P, I>::iterator > its;

      S(I k, W c, const vector< item_t<W, P> > &items) {
        if (k > 0) {
          empty = false;
          // Combine the lazy functions. This is the algorithm described at
          // "Sparse knapsack algo-tech-cuit and its synthesis", all the rest
          // is only implementation of the lazy evaluation in c++.
          s_pred_k = unique_ptr< S<W, P, I> >(new S<W, P, I>(k-1, c, items));
          l_items = Lazyfy<W, P>(items);
          item_t<W, P> zero = {0, 0};
          addhead = AddHead<W, P>(zero, this->begin());
          addtest = AddTest<W, P>(items[k-1], &addhead, c);
          merge = Merge<W, P>(s_pred_k->begin(), &addtest);
          filter = Filter<W, P>(&merge, 0);
        } else {
          empty = true;
        }
      }

      bool has_next(void) {
        if (empty) return false;

        item_t<W, P> next;
        empty = !filter.get_next(next);
        if (!empty) computed.push_back(next);
        else { // Is this correct?
          computed.clear();
          computed.shrink_to_fit();
        }

        return !empty;
      }

      iterator * begin(void) {
        iterator tmp(this);
        its.push_front(tmp);
        return &its.front();
      }

      void garbage_collect(void) {
        auto it = its.cbegin();
        size_t m = it->ix;
        ++it;

        for (; it != its.cend(); ++it) {
          m = min(m, it->ix);
        }

        // Another possibility is to verify if (m > computed.size()/MAGIC_CONST)
        if (m > HBM_MAX_MEMORY_WASTED_BY_S) {
          vector< item_t<W, P> > new_v(computed.cbegin()+m, computed.cend());
          computed.swap(new_v);

          for (auto it = its.begin(); it != its.end(); ++it) {
            it->ix = it->ix - m;
          }
        }
      }
    };

    /// High level wrapper that makes eduk work with the instance_t and
    /// solution_t types, calls S(I k, W c, const vector< item_t<W, P> >
    /// &items).
    template<typename W, typename P, typename I>
    void eduk(instance_t<W, P> &ukpi, solution_t<W, P, I> &sol, bool already_sorted = false) {
      // Extra Info Pointer == eip
      eduk_extra_info_t<W, P, I>* eip = new eduk_extra_info_t<W, P, I>();
      extra_info_t* upcast_ptr = dynamic_cast<extra_info_t*>(eip);
      sol.extra_info = std::shared_ptr<extra_info_t>(upcast_ptr);

      I n = ukpi.items.size();
      W c = ukpi.c;
      vector< item_t<W, P> > &items(ukpi.items);
      if (!already_sorted) sort_by_weigth(ukpi.items);

      S<W, P, I> s = S<W, P, I>(n, c, items);
      auto it = s.begin();

      vector< item_t<W, P> > res;
      consume(it, res);

      sol.opt = res.back().p;
      sol.y_opt = res.back().w;
    }

    
    template<typename W, typename P, typename I>
    struct eduk_wrap : wrapper_t<W, P, I> {
      virtual void operator()(instance_t<W, P> &ukpi, solution_t<W, P, I> &sol, bool already_sorted) const {
        // Calls the overloaded version with the third argument as a bool
        hbm_eduk_impl::eduk(ukpi, sol, already_sorted);

        return;
      }

      virtual const std::string& name(void) const {
        static const std::string name = "eduk";
        return name;
      }
    };

    template<typename W, typename P, typename I = size_t>
    void eduk(instance_t<W, P> &ukpi, solution_t<W, P, I> &sol, int argc, argv_t argv) {
      simple_wrapper(eduk_wrap<W, P, I>(), ukpi, sol, argc, argv);
    }
  }

  /// Solves the UKP instance in ukpi and stores results at sol.
  /// If already_sorted is false, then the item list will be sorted by weight.
  /// If already_sorted is true, then the code will expect the items to be
  /// ordered by weight (and will fail if they aren't).
  ///
  /// @param ukpi An UKP instance.
  /// @param sol A object that will be overwritten to store the results.
  /// @param already_sorted If the ukpi.items vector needs to be sorted by
  ///   weight, or not.
  template<typename W, typename P, typename I = size_t>
  void eduk(instance_t<W, P> &ukpi, solution_t<W, P, I> &sol, bool already_sorted = false) {
    hbm_eduk_impl::eduk(ukpi, sol, already_sorted);
  }

  /// An overloaded function, it's used as argument to test_common functions.
  ///
  /// The only parameter recognized is "--already-sorted". If this parameter is
  /// given the ukpi.items isn't sorted by non-decreasing weight. If it's
  /// ommited the ukpi.items is sorted by non-decreasing weight.
  ///
  /// @see main_take_path
  /// @see eduk(instance_t<W, P> &, solution_t<W, P, I> &, bool)
  template<typename W, typename P, typename I = size_t>
  void eduk(instance_t<W, P> &ukpi, solution_t<W, P, I> &sol, int argc, argv_t argv) {
    hbm_eduk_impl::eduk(ukpi, sol, argc, argv);
  }
}

#endif //HBM_EDUK_HPP
