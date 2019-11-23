/*###############################################
###   Authors: {Michael Lynch,                ###
###                           }               ###
###                                           ###
###   Project: ECE621 Class Project           ###
###                                           ###
###   Name: expt_main                         ###
###                                           ###
###   Purpose: Provide a driver for the       ###
###            sim-safe experiments for       ###
###            various branch predictors and  ###
###            benchmarks                     ###
###                                           ### 
#################################################
###############################################*/

#include <iostream>
#include <vector>
#include <fstream>
#include <ios>
#include <string>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

std::vector<std::vector<std::string>> genSpec2K6Expts()
{
  std::vector<std::vector<std::string>> experiments;
  std::vector<std::string> perceptron{"../benchmarks/sim-bpred", "-bpred", "perceptron", "-bpred:perceptron"};
  std::vector<std::string> twolev{"../benchmarks/sim-bpred", "-bpred", "2lev", "-bpred:2lev"};
  std::vector<std::string> nottaken{"../benchmarks/sim-bpred", "-bpred", "nottaken"};
  std::vector<std::string> taken{"../benchmarks/sim-bpred", "-bpred", "taken"};
  std::string expt_dir = "../benchmarks/spec2k6-alpha-bin";

  std::vector<std::vector<std::string>> benchmarks;

  benchmarks = {
		{expt_dir+"/gobmk/gobmk.base.alpha", expt_dir+"/gobmk/test/input/dragon1.sgf", "gobmk"},
		{expt_dir+"/gromacs/gromacs.base.alpha", "-silent", "-deffnm", expt_dir+"/gromacs/test/gromacs.tpr", "-nice", "0", "gromacs"},
		{expt_dir+"/hmmer/hmmer.base.alpha", expt_dir+"/hmmer/test/input/bombesin.hmm", "hmmer"},
		{expt_dir+"/mcf/mcf.base.alpha", expt_dir+"/mcf/test/input/inp.in", "mcf"},
	       // {expt_dir+"/milc/milc.base.alpha", "<", expt_dir+"/test/su3imp.in", "milc"},
		{expt_dir+"/sjeng/sjeng.base.alpha", expt_dir+"/sjeng/test/input/test.txt", "sjeng"}};

  std::vector<std::vector<unsigned int>> expt_setup{{1,7,12},{2,9,22},{4,11,28},{8,13,34},{16,14,36},
					   {32,15,59},{64,16,59},{128,17,62},{256,17,62},{512,19,62}};
  bool control_expts = false;
  for (auto& expt_cfg : expt_setup)
    {
      int hw_budget = expt_cfg[0];
      int hist_2lev = expt_cfg[1];
      int hist_perc = expt_cfg[2];
      for (auto& benchmark : benchmarks)
	{
	  std::string benchmark_prog = benchmark.front();
	  std::vector<std::string> benchmark_args(++benchmark.begin(), --benchmark.end());
	  std::string benchmark_tag = benchmark.back();
	  std::vector<std::string> this_expt_perc;
	  std::vector<std::string> this_expt_twolev;
	  std::vector<std::string> this_expt_taken;
	  std::vector<std::string> this_expt_nottaken;

	  // boilerplate
	  this_expt_perc.insert(this_expt_perc.end(), perceptron.begin(), perceptron.end());
	  this_expt_twolev.insert(this_expt_twolev.end(), twolev.begin(), twolev.end());
	  this_expt_taken.insert(this_expt_taken.end(), taken.begin(), taken.end()); 
	  this_expt_nottaken.insert(this_expt_nottaken.end(), nottaken.begin(), nottaken.end());
	  
	  //percept config
	  this_expt_perc.push_back(std::to_string(hist_perc));
	  this_expt_perc.push_back(std::to_string(0));
	  this_expt_perc.push_back(std::to_string(1024));
	  // 2lev config
	  this_expt_twolev.push_back(std::to_string(1));
	  this_expt_twolev.push_back(std::to_string(1024));
	  this_expt_twolev.push_back(std::to_string(hist_2lev));
	  this_expt_twolev.push_back(std::to_string(0));

	  // redir
	  this_expt_perc.push_back("-redir:sim");
	  this_expt_twolev.push_back("-redir:sim");
	  this_expt_taken.push_back("-redir:sim");
	  this_expt_nottaken.push_back("-redir:sim");
	  
	  // benchmark config
	  this_expt_perc.push_back(benchmark_prog);
	  this_expt_twolev.push_back(benchmark_prog);
	  this_expt_taken.push_back(benchmark_prog);
	  this_expt_nottaken.push_back(benchmark_prog);

	  for (auto arg : benchmark_args)
	    {
	      this_expt_perc.push_back(arg);
	      this_expt_twolev.push_back(arg);
	      this_expt_taken.push_back(arg);
	      this_expt_nottaken.push_back(arg);
	    }
	  // NULL and expt label
	  this_expt_perc.push_back("NULL");
	  this_expt_twolev.push_back("NULL");
	  this_expt_taken.push_back("NULL");
	  this_expt_nottaken.push_back("NULL");
	  
	  this_expt_perc.push_back("HW:"+std::to_string(hw_budget)+"-perceptron-"+benchmark_tag);
	  this_expt_twolev.push_back("HW:"+std::to_string(hw_budget)+"-2lev-"+benchmark_tag);
	  this_expt_taken.push_back("control-taken-"+benchmark_tag);
	  this_expt_nottaken.push_back("control-nottaken-"+benchmark_tag);
	  
	  // add to experiments
	  if (!control_expts)
	    {
	      experiments.push_back(this_expt_taken);
	      experiments.push_back(this_expt_nottaken);
	    }
	  experiments.push_back(this_expt_twolev);
	  experiments.push_back(this_expt_perc);
	}
      control_expts = true;
    }
  return experiments;
}

std::vector<std::vector<std::string>> genPerceptronExpts()
{
  std::vector<std::vector<std::string>> experiments;
  std::vector<std::string> ex_name{"../benchmarks/sim-bpred", "-bpred", "perceptron", "-bpred:perceptron"};
  std::vector<std::string> benchmarks{"../benchmarks/cc1.alpha", "-O", "../benchmarks/1stmt.i", "NULL"};


  for (int global_idx = 36; global_idx<37; global_idx++)
    {
      for (int local_idx = 0; local_idx<100; local_idx+=5)
	{
	  std::vector<std::string> this_expt;
	  
	  this_expt.insert(this_expt.end(), ex_name.begin(), ex_name.end()); // add sim-bpred -bpred:perceptron
	  this_expt.push_back(std::to_string(global_idx));
	  this_expt.push_back(std::to_string(local_idx));
	  this_expt.push_back(std::to_string(1024));
	  this_expt.push_back("-redir:sim");
	  this_expt.insert(this_expt.end(), benchmarks.begin(), benchmarks.end()); // add benchmark info
	  this_expt.push_back("perceptron:("+std::to_string(global_idx)+","+std::to_string(local_idx)+")");
	  experiments.push_back(this_expt);
	}
    }
  return experiments;
}


int main()
{

  std::vector<std::vector<std::string>> experiments = genSpec2K6Expts();
  
  unsigned int MAX_CONCURRENT_JOBS = 2; // how many expts we want running at once max: like a batch, start x, wait for x
  unsigned int JOBS_RUNNING = 0; // how many jobs are currently active 
  unsigned int WAITING_ON = 0; // the job we are currently waiting on
  
  std::cerr << "[*] Welcome to the main driver program" << std::endl
	    << "[*] This program will fork each experiment and consolidate the results into a file."
	    << std::endl;

  std::vector<pid_t> child_pid_vec;
  std::vector<std::string> tmpname_vec;
  std::vector<std::string> expt_tag_vec;

  /* Loop over experiments and exec each one in a new process, passing the tempfile as an arg */
  for (auto arg_vector : experiments)
    {

      if (JOBS_RUNNING>=MAX_CONCURRENT_JOBS)
	{
	  std::cerr << "[*] Job concurrency limit reached. Waiting for the oldest job to finish." << std::endl;
	  pid_t child_pid = child_pid_vec[WAITING_ON]; // pid of the first unfinished job
	  waitpid(child_pid, NULL, 0); // wait for it to finish
	  std::cerr << "[*] Job with PID " << child_pid << " finished." << std::endl;
	  WAITING_ON++; // move to next unfinished job
	  JOBS_RUNNING--; // decrement running jobs
	}

      char c_tmpname[] = "/tmp/ece621-tempXXXXXX";
      int tmpfd = mkstemp(c_tmpname); close(tmpfd); // we only care about the filename
      std::string tmpname (c_tmpname);
      tmpname_vec.push_back(tmpname); // add c_tmpname to array to maintain order
      
      std::vector<char const *> expt_arg_vec; // vec of chars to hold our args
      char * const* expt_argv; // to hold the data of expt_arg_vec

      for (int arg_idx = 0; arg_idx < arg_vector.size(); arg_idx++)
	{
	  if (arg_vector[arg_idx]=="NULL") // if we hit NULL, args are full
	    {
	      expt_arg_vec.push_back(nullptr);
	      std::string expt_tag = arg_vector[arg_idx+1];
	      expt_tag_vec.push_back(expt_tag);
	      break;
	    }
	  else
	    {
	      expt_arg_vec.push_back(arg_vector[arg_idx].data());
	      
	      if (arg_vector[arg_idx].compare("-redir:sim")==0) // if arg is output redir
		{
		  expt_arg_vec.push_back(tmpname_vec.back().data()); // add tmpfile name as arg
		}
	    }
	}
      pid_t child_proc_pid;
     
      expt_argv = const_cast <char **> (expt_arg_vec.data());
      std::cerr << "[*] Starting experiment: " << arg_vector.end()[-1] << std::endl;
      
      child_proc_pid = fork();
      if (child_proc_pid<0)
	{ //something went wrong in the fork
	  std::cerr << "[!!!] Did not fork new process! Exiting." << std::endl; 
	  exit(1);
	}
      else if(child_proc_pid>0)
	{
	  JOBS_RUNNING++;
 	  child_pid_vec.push_back(child_proc_pid);
	  std::cerr << "[*] Forked with PID " << child_proc_pid << std::endl;

	}
      else
	{
	  
	  if(execvp(expt_argv[0], expt_argv)){return 1;}
	}
    }
  
  std::cerr << "[*] All experiments dispatched." << std::endl;
  if (WAITING_ON<child_pid_vec.size())
    {
      std::cerr << "[*] Waiting for " << child_pid_vec.size()-WAITING_ON
		<< " jobs to finish." << std::endl; 
      for (WAITING_ON; WAITING_ON<child_pid_vec.size(); WAITING_ON++)
	{
	  pid_t child_pid = child_pid_vec[WAITING_ON];
	  waitpid(child_pid, NULL, 0);
	}
    }
  std::cerr << "[*] All experiments finished." << std::endl
	    << "[*] Consolidating data..." << std::endl
    ;
  /*consolidate tempfiles into one and give to python */
  char c_data[] = "./ece621-python.data";
  int python_temp = mkstemp(c_data); close(python_temp); // will use ofstream
  std::ofstream python_file(c_data,
			    std::ios_base::app);
  
  for (int i=0; i<tmpname_vec.size(); i++) //(auto tempfile : tmpname_vec)
    {
      std::string tempfile = tmpname_vec[i];
      std::ifstream fs_tempfile(tempfile.data());
				//,std::ios_base::binary);
      //std::istream ss_expt_tag(expt_tag_vec[i]);
      
      python_file << "EXPT_BEGIN" << std::endl
		  << expt_tag_vec[i] << std::endl
		  << fs_tempfile.rdbuf()
		  << "EXPT_END"<< std::endl;
    }
  /*------------------------------------------------------*/
  // Cleanup and quit
    
  std::cerr << "[*] Cleaning up tempfiles..." << std::endl;
  /* cleanup tempfiles */
  for (auto tempfile : tmpname_vec)
    {
      std::remove(tempfile.data());
    }
  
  std::cerr << "[*] Done!" << std::endl;
  return(0);
}
