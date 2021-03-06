# batch_experiment

Things you could want to do, and this tool does for you:

* You want to run a batch of sh commands, but only one of those per core/cpu.
* You want to maximize the core use, the moment a core/cpu becomes free from one of your commands, you want the next command to take its place.
* You want the output of those commands to be saved by command/file (you want to have a file with what exactly the command outputted).
* You want to specify timeouts for those commands to be killed.
* You want the power to resume the batch from an interruption (i.e. system crashes, or energy goes down) without losing any work.

What conditions you need to use this tool:

* You use linux.
* You have installed: sh (the shell); ruby (obvioulsy); time (NOT the bash/sh internal command, but the [package one](https://www.archlinux.org/packages/extra/x86_64/time/)); timeout (from coreutils); taskset (from [util-linux](https://www.archlinux.org/packages/core/x86_64/util-linux/)).

What is not needed:

* To know how to program in ruby. Only taking less than 5 minutes to learn some basic syntax will suffice to run commands on multiple cores and save the results to files (using BatchExperiment::batch). However, if you want not only to execute the commands but want to extract and group some information from their output to a CSV (using BatchExperiment::experiment), you will need to tell ruby how to do the extracting part.

## How to use it

You will need to create a ruby script (copy, past and adapt one of the provided examples), give it execution permissions ("chmod +x script.rb"), and execute it ("./script.rb"). It's good to remember that, if you didn't created a folder for the output files and set the :output_dir configuration to it, the script will probably flood your current folder with the output files. Also, good to remember that, if you didn't set the :cwd configuration, the commands will be called from the folder where you called the script, so they probably will expect that any relative paths/filenames given to them to be relative to the current folder.

## Examples

After installing the gem, you will have a examples folder (/home/YOUR_USER/.gem/ruby/RUBY_VERSION/gems/batch_experiment-GEM_VERSION/examples). The sample_batch.rb gives you a good ideia of how to use ::batch (no csv creation).

```ruby
#!/bin/ruby

require 'batch_experiment'

commands = [
  'sleep 2 && echo orange',
  'sleep 4 && echo banana',
  'sleep 100 && echo "never gonna happen"', # killed by timeout
]

conf = {
  # IDs of the CPU cores that can be used for executing tests.
  cpus_available: [0, 1],
  # Maximum number of seconds that a command can run. After this a kill command
  # (TERM signal) will be issued.
  timeout: 5,
  # Maximum number of seconds that a command can run after a kill command was
  # issued. After this a kill -9 command (KILL signal) will be issued.
  post_timeout: 1,
}

BatchExperiment::batch(commands, conf)
```

The experiment_example.rb (and the lib/batch_experiment/sample_extractors.rb) gives a good ideia of how to use #experiment with multiple commands and how to create an extractor (used to create a csv).

This code was born in [this repository](https://github.com/henriquebecker91/masters/tree/master/codes/rb/batch_experiment).

