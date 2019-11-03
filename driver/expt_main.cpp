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
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>

// forks the process 
pid_t fork_to_tempfile(int pipe_fd[2], std::FILE* tempfile)//, char *arg_array[])
{
  pid_t child_proc = fork();
  if(child_proc>0) // if we are in child process
    {
      dup2(pipe_fd[1], fileno(tempfile));
      close(pipe_fd[0]);
      close(pipe_fd[1]);
    }
  else // if we are parent
    {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
    }
  
  return (child_proc);
}

int main()
{
  // placeholder array for now. will be populated with
  //{"sim-safe", "-perceptpred", "go.alpha", <GO_ARGS>, NULL, expt#, expt_desc} for example
  std::vector<std::vector<std::string>> experiments{
						     {"name", "s1", "s2", "s3", "benchmark", "benchmark_args", "END_ARGS", "1", "Test expt1"},
						     {"name2", "s12", "s22", "s32", "benchmark2", "benchmark_args2", "END_ARGS", "2", "Test expt 2"}};
  
  std::cerr << "[*] Welcome to the main driver program.\n"
	    << "[*] This program will fork each experiment and consolidate the results into a file."
	    << std::endl;

  std::vector<pid_t> child_pid_vec;
  std::vector<std::FILE*> tempfile_vec;
  int pipe_fd[2];

  /* Loop over experiments and exec each one in a new thread, passing the tempfile 
     to fork_to_tempfile */
  for (auto arg_vector : experiments)
    {
      std::vector<char *> expt_arg_vec;
      char **expt_argv;
      for (std::string arg_str : arg_vector)
	{
	  std::vector<char> arg(arg_str.c_str(), arg_str.c_str()+arg_str.size()+1);
	  if (arg_str=="END_ARGS")
	    {
	      expt_arg_vec.push_back(&arg[0]);
	      break;
	    }
	}
	
      pid_t child_proc_pid;
      std::FILE* tmpf = std::tmpfile(); //create tempfile for process
      tempfile_vec.push_back(tmpf); // add tempfile to array to maintain order
      pipe(pipe_fd); // create pipe 
      expt_argv = expt_arg_vec.data();
      std::cerr << "[*] Starting experiment " << arg_vector.end()[-2]
		<< ": " << arg_vector.end()[-1] << std::endl;
      
      child_proc_pid = fork_to_tempfile(pipe_fd, tmpf);
      if (child_proc_pid<0)
	{ //something went wrong in the fork
	  //cleanup temp files
	  std::cerr << "[!!!] Did not fork new process! Exiting." << std::endl; 
	  exit(1);
	}
      else if(child_proc_pid>0)
	{
 	  child_pid_vec.push_back(child_proc_pid);
	  std::cerr << "[*] Forked with PID " << child_proc_pid << std::endl;
	}
      else{
	execv(expt_argv[0], expt_argv);
      }
    }
  /*consolidate tempfiles into one */
  
  
  /* cleanup tempfiles */
  for (auto tempfile : tempfile_vec)
    {
      std::fclose(tempfile);
    }
  
  
  return(0);
}
