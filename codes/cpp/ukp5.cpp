#include "ukp5.hpp"

#if (defined(HBM_CHECK_PERIODICITY) || defined(HBM_CHECK_PERIODICITY_FAST)) && (defined(HBM_INT_EFF) || defined(HBM_FP_EFF))
  #error CHECK PERIODICITY ONLY MAKE SENSE IF THE ORDER IS NOT AN APPROXIMATION
#endif
#if defined(HBM_CHECK_PERIODICITY) && defined(HBM_CHECK_PERIODICITY_FAST)
  #error ONLY ONE OF HBM_CHECK_PERIODICITY OR HBM_CHECK_PERIODICITY_FAST CAN BE DEFINED AT THE SAME TIME
#endif

using namespace std;
#ifdef HBM_PROFILE
#include <chrono>
using namespace std::chrono;
#endif

using namespace hbm;

pair<weight,weight> minmax_item_weight(vector<item> &items) {
  weight min, max;
  min = max = items[0].w;
  for (auto it = items.begin()+1; it != items.end(); ++it) {
    weight x = it->w;
    if (x < min) min = x;
    else if (x > max) max = x;
  }
  return make_pair(min,max);
}

#ifdef HBM_PROFILE
void ukp5_gen_stats(weight c, itemix n, weight w_min, weight w_max, const vector<profit> &g, const vector<itemix> &d, solution &sol) {
  sol.g = g;
  sol.d = d;
  sol.c = c;
  sol.n = n;
  sol.w_min = w_min;
  sol.w_max = w_max;

  sol.non_skipped_d.assign(c-w_min + 1, n-1);

  sol.qt_i_in_dy.assign(n, 0);
  sol.qt_i_in_dy[n-1] = w_min;

  sol.qt_gy_zeros = w_min;
  profit opt = 0;
  sol.qt_non_skipped_ys = 0;
  sol.qt_inner_loop_executions = 0;

  for (weight y = w_min; y <= c-w_min; ++y) {
    ++sol.qt_i_in_dy[d[y]];
    if (g[y] > opt) {
      ++sol.qt_non_skipped_ys;
      sol.qt_inner_loop_executions += d[y];
      sol.non_skipped_d[y] = d[y];
      opt = g[y];
    }
    if (g[y] == 0) ++sol.qt_gy_zeros;
    if (d[y] != 0 && d[y] != (n-1)) sol.last_dy_non_zero_non_n = y;
  }
  return;
}
#endif

pair<profit, weight> get_opts(weight c, const vector<profit> &g, weight w_min) {
  profit opt = 0;
  /* Dont need to be initialized, but initializing to stop compiler
   * warning messages */
  weight y_opt = 0;

  for (weight y = (c-w_min)+1; y <= c; ++y) {
    if (opt < g[y]) {
      opt = g[y];
      y_opt = y;
    }
  }

  return make_pair(opt, y_opt);
}

void ukp5_phase2(const vector<item> &items, const vector<itemix> &d, solution &sol) {
  itemix n = items.size();
  vector<itemix> qts_its(n, 0);

  weight y_opt = sol.y_opt;
  itemix dy_opt;
  while (y_opt != 0) {
    dy_opt = d[y_opt];
    y_opt -= items[dy_opt].w;
    ++qts_its[dy_opt];
  }

  for (itemix i = 0; i < n; ++i) {
    if (qts_its[i] > 0) {
      sol.used_items.emplace_back(items[i], qts_its[i], i);
    }
  }
  sol.used_items.shrink_to_fit();

  return;
}

void ukp5_phase1(const instance &ukpi, vector<profit> &g, vector<itemix> &d, solution &sol, weight w_min, weight w_max) {
  const weight &c = ukpi.c;
  const itemix &n = ukpi.items.size();
  const vector<item> &items = ukpi.items;

  weight &y_opt = sol.y_opt;
  profit &opt = sol.opt;

  opt = 0;

  #if defined(HBM_CHECK_PERIODICITY) || defined(HBM_CHECK_PERIODICITY_FAST)
  weight  last_y_where_nonbest_item_was_used = 0;
  #endif
  #ifdef HBM_CHECK_PERIODICITY_FAST
  /* We could pre-compute the maximum weight at each point
   * to accelerate the periodicity check, but this would make
   * the looser, and the periodicity check doesn't seem to
   * consume much of the algorithm time
   */
  vector<weight> w_maxs;
  w_maxs.reserve(n);
  w_maxs.push_back(items[0].w);
  for (itemix i = 1; i < n; ++i) {
    w_maxs.push_back(max(w_maxs[i-1], items[i].w));
  }
  #endif

  /* this block is a copy-past of the loop bellow only for the best item
   * its utility is to simplify the code when HBM_CHECK_PERIODICITY is defined
   */
  weight wb = items[0].w;
  g[wb] = items[0].p;;
  d[wb] = 0;

  #ifdef HBM_CHECK_PERIODICITY_FAST
  last_y_where_nonbest_item_was_used = w_max;
  #endif
  for (weight i = 0; i < n; ++i) {
    profit pi = items[i].p;
    weight wi = items[i].w;
    if (g[wi] < pi) {
      g[wi] = pi;
      d[wi] = i;
      #ifdef HBM_CHECK_PERIODICITY
      if (wi > last_y_where_nonbest_item_was_used) {
        last_y_where_nonbest_item_was_used = wi;
      }
      #endif
    }
  }

  opt = 0;
  for (weight y = w_min; y <= c-w_min; ++y) {
    if (g[y] <= opt) continue;
    #if defined(HBM_CHECK_PERIODICITY) || defined(HBM_CHECK_PERIODICITY_FAST)
    if (last_y_where_nonbest_item_was_used < y) break;
    #endif

    profit gy;
    itemix dy;
    opt = gy = g[y];
    dy = d[y];

    #ifdef HBM_CHECK_PERIODICITY_FAST
    if (dy != 0) last_y_where_nonbest_item_was_used = y + w_maxs[dy];
    #endif

    /* this block is a copy-past of the loop bellow only for the best item
     * its utility is to simplify the code when HBM_CHECK_PERIODICITY is defined
     */
    item bi = items[0];
    weight wb = bi.w;
    profit pb = bi.p;
    weight next_y = y + wb;
    profit old_gny = g[next_y];
    profit new_gny = gy + pb;
    if (old_gny < new_gny) {
      g[next_y] = new_gny;
      d[next_y] = 0;
    }

    for (itemix ix = 1; ix <= dy; ++ix) {
      item it = items[ix];
      weight wi = it.w;
      profit pi = it.p;
      weight ny = y + wi;
      profit ogny = g[ny];
      profit ngny = gy + pi;
      if (ogny < ngny) {
        g[ny] = ngny;
        d[ny] = ix;
        #ifdef HBM_CHECK_PERIODICITY
        if (ny > last_y_where_nonbest_item_was_used) last_y_where_nonbest_item_was_used = ny;
        #endif
      }
    } 
  }

  #if defined(HBM_CHECK_PERIODICITY) || defined(HBM_CHECK_PERIODICITY_FAST)
  if (last_y_where_nonbest_item_was_used < c-w_min) {
    weight y_ = last_y_where_nonbest_item_was_used;
    while (d[y_] != 0) ++y_;

    weight a1 = items[0].w;
    weight extra_capacity = c - y_;

    weight space_used_by_best_item = extra_capacity - (extra_capacity % a1);

    auto opts = get_opts(c-space_used_by_best_item, g, w_max);
    opt = opts.first;
    y_opt = opts.second;
    #ifdef HBM_PROFILE
    sol.last_y_value_outer_loop = last_y_where_nonbest_item_was_used+1;
    #endif
  } else {
    auto opts = get_opts(c, g, w_min);
    opt = opts.first;
    y_opt = opts.second;
    #ifdef HBM_PROFILE
    sol.last_y_value_outer_loop = c-w_min;
    #endif
  }
  #else
  auto opts = get_opts(c, g, w_min);
  opt = opts.first;
  y_opt = opts.second;
  #endif

  return;
}

/* This function reorders the ukpi.items vector, if you don't want this pass a
 * copy of the instance or pass it already ordered by non-decreasing eff
 * and true for the parameter already_sorted.
 */
void hbm::ukp5(instance &ukpi, solution &sol, bool already_sorted/* = false*/) {
  #ifdef HBM_PROFILE
  steady_clock::time_point all_ukp5_begin = steady_clock::now();
  steady_clock::time_point begin = steady_clock::now();
  #endif
  if (!already_sorted) sort_by_eff(ukpi.items);
  #ifdef HBM_PROFILE
  sol.sort_time = duration_cast<duration<double>>(steady_clock::now() - begin).count();
  #endif

  #ifdef HBM_PROFILE
  begin = steady_clock::now();
  #endif
  weight c = ukpi.c;
  itemix n = ukpi.items.size();
  auto minw_max = minmax_item_weight(ukpi.items);
  weight w_min = minw_max.first, w_max = minw_max.second;
  #ifdef HBM_PROFILE
  sol.linear_comp_time = duration_cast<duration<double>>(steady_clock::now() - begin).count();
  #endif

  #ifdef HBM_PROFILE
  begin = steady_clock::now();
  #endif

  #ifdef HBM_PROFILE
  /* Use the solution fields instead of local variables to propagate
   * the array values. The arrays will be dumped to files, making possible
   * study them with R or other tool. */
  vector<profit> &g = sol.g;
  vector<itemix> &d = sol.d;
  g.assign(c+1+(w_max-w_min), 0);
  d.assign(c+1+(w_max-w_min), n-1);
  #else
  vector<profit> g(c+1+(w_max-w_min), 0);
  vector<itemix> d(c+1+(w_max-w_min), n-1);
  #endif

  #ifdef HBM_PROFILE
  sol.vector_alloc_time = duration_cast<duration<double>>(steady_clock::now() - begin).count();
  #endif

  #ifdef HBM_PROFILE
  begin = steady_clock::now();
  #endif
  ukp5_phase1(ukpi, g, d, sol, w_min, w_max);
  #ifdef HBM_PROFILE
  sol.phase1_time = duration_cast<duration<double>>(steady_clock::now() - begin).count();
  begin = steady_clock::now();
  #endif
  ukp5_phase2(ukpi.items, d, sol);
  #ifdef HBM_PROFILE
  sol.phase2_time = duration_cast<duration<double>>(steady_clock::now() - begin).count();
  sol.total_time = duration_cast<duration<double>>(steady_clock::now() - all_ukp5_begin).count();
  #endif

  #ifdef HBM_PROFILE
  ukp5_gen_stats(c, n, w_min, w_max, g, d, sol);
  #endif

  #if defined(HBM_CHECK_PERIODICITY) || defined(HBM_CHECK_PERIODICITY_FAST)
  /* If we use our periodicity check the sol.used_items constructed by
   * ukp5_phase2 doesn't include the copies of the best item used to
   * fill all the extra_capacity. This solves it. */
  weight qt_best_item_inserted_by_periodicity = (c - sol.y_opt)/ukpi.items[0].w;
  sol.opt += qt_best_item_inserted_by_periodicity*ukpi.items[0].p;
  sol.y_opt+= qt_best_item_inserted_by_periodicity*ukpi.items[0].w;
  /* Our periodicity check will always get a partial solution that have at
   * least one of the best item. And ukp5_phase2 will populate the
   * sol.used_items in the same order the sol.items are, this way we
   * have the guarantee that the first element of sol.used_items
   * exist and it's the best item. */
  sol.used_items[0].qt += qt_best_item_inserted_by_periodicity;
  #endif

  return;
}

