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

int main()
{
  // placeholder array for now. will be populated with
  //{"sim-safe", "-perceptpred", "go.alpha", <GO_ARGS>, NULL, expt#, expt_desc} for example
  std::vector<std::vector<std::string>> experiments{
						    {"sim-bpred", "-bpred", "nottaken", "-redir:sim", "cc1.alpha", "-O", "1stmt.i", "NULL", "1", "control-nottaken"},
						     {"sim-bpred", "-bpred", "2lev", "s32","-redir:sim", "cc1.alpha", "-O", "1stmt.i", "NULL", "2", "2-lev"},
						    {"sim-bpred", "-bpred", "perceptron", "36", "0", "1024","-redir:sim", "cc1.alpha", "-O", "1stmt.i", "NULL", "3", "perceptron{}"}};
  
  unsigned int MAX_CONCURRENT_JOBS = 10; // how many expts we want running at once max: like a batch, start x, wait for x
  unsigned int JOBS_RUNNING = 0; // how many jobs are currently active 
  unsigned int WAITING_ON = 0; // the job we are currently waiting on
  
  std::cerr << "[*] Welcome to the main driver program" << std::endl
	    << "[*] This program will fork each experiment and consolidate the results into a file."
	    << std::endl;

  std::vector<pid_t> child_pid_vec;
  std::vector<std::string> tmpname_vec;

  /* Loop over experiments and exec each one in a new process, passing the tempfile as an arg */
  for (auto arg_vector : experiments)
    {
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
	      break;
	    }
	  else
	    {
	      expt_arg_vec.push_back(arg_vector[arg_idx].data());
	      
	      if (arg_vector[arg_idx] == "-redir:sim") // if arg is output redir
		{
		  expt_arg_vec.push_back(tmpname_vec.back().data()); // add tmpfile name as arg
		}
	    }
	}
      pid_t child_proc_pid;

      expt_argv = const_cast <char **> (expt_arg_vec.data());
      std::cerr << "[*] Starting experiment " << arg_vector.end()[-2]
		<< ": " << arg_vector.end()[-1] << std::endl;
      
      child_proc_pid = fork();
      if (child_proc_pid<0)
	{ //something went wrong in the fork
	  std::cerr << "[!!!] Did not fork new process! Exiting." << std::endl; 
	  exit(1);
	}
      else if(child_proc_pid>0)
	{
 	  child_pid_vec.push_back(child_proc_pid);
	  std::cerr << "[*] Forked with PID " << child_proc_pid << std::endl;
	  JOBS_RUNNING++;
	  if (JOBS_RUNNING>MAX_CONCURRENT_JOBS)
	    {
	      std::cerr << "[*] Job concurrency limit reached. Waiting for the oldest job to finish." << std::endl;
	      pid_t child_pid = child_pid_vec[WAITING_ON]; // pid of the first unfinished job
	      waitpid(child_pid, NULL, 0); // wait for it to finish
	      std::cerr << "[*] Job with PID " << child_pid << " finished." << std::endl;
	      WAITING_ON++; // move to next unfinished job
	      JOBS_RUNNING--; // decrement running jobs
	    }

	}
      else
	{
	  if(execvp(expt_argv[0], expt_argv)){return 1;}
	}
    }
  
  std::cerr << "[*] All experiments dispatched." << std::endl;
  if (WAITING_ON<child_pid_vec.size()-1)
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
  char c_data[] = "./ece621-pythonXXXXXX.data";
  int python_temp = mkstemp(c_data); close(python_temp); // will use ofstream
  std::ofstream python_file(c_data,
			    std::ios_base::app |
			    std::ios_base::binary);
  
  for (auto tempfile : tmpname_vec)
    {
      std::ifstream fs_tempfile(tempfile.data(),
				std::ios_base::binary);
      python_file << fs_tempfile.rdbuf();
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
