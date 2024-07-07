#include "systemcalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/

  printf("[KDBG] Entering function do_system()\r\n") ;

  printf("[KBDG] Calling function system( '%s' )\r\n",cmd) ;
  int return_value = system( cmd ) ;
  printf("[KDBG] ... it returned %d\r\n",return_value) ;

  if( return_value == 0 )
  {
    printf("[KDBG] ... do_system() return true\r\n") ;
    return true ;
  }
  else
  {
    printf("[KDBG] ... do_system() return false\r\n") ;
    return false ;
  }

  //  return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
  printf("[KDBG] Entering function do_exec()\r\n") ;

    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
      printf("[KDBG] command[%d] = '%s'\r\n",i,command[i]) ;
    }
    command[count] = NULL;

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

  fflush( stdout ) ; // To prevent some double printing by printf()

  pid_t fork_pid = fork() ;

  // Check for error
  if( fork_pid == (-1) )
  {
    printf("[KDBG] Error in fork()\r\n") ;
    return false ;
  }

  printf("[KDBG] fork_pid = %d\r\n",fork_pid) ;

  bool child_failed_flag = false ;
  bool return_value = true ;

  switch( fork_pid )
  {
    case (-1) : // Error !!!
      {
        perror("Error in fork()") ;
        printf("[KDBG] Error in fork()") ;
        return_value = false ;
        return false ;
      }
      break ; // This 'break' is unnecessary, but I want to be consistant
    case 0 : // Child
      {
        //int execv(const char *path, char *const argv[]);
        printf("[KDBG] Calling function execv( '%s' , ... )\r\n",command[0]) ;
        int execv_return_value = execv( command[0] , command ) ; // Should not return from this, if things work, will return at the 'wait()'
        printf("[KDBG] execv() return value = %d\r\n",execv_return_value) ;
        //execv( command[0] , command ) ; // Should not return from this, if things work, will return at the 'wait()'
        //int execv_return_value = execv( command[0] , &command[1] ) ;
        //char *kargs[2] = {"-lat","-r"} ;
        //int execv_return_value = execv( "/usr/bin/ls" , kargs ) ;
        //char *kargs[2] = {"/usr/bin/touch","ken_was_here.txt"} ;
        //int execv_return_value = execv( "/usr/bin/touch" , kargs ) ;
        if( execv_return_value != 0 ) // Should be (-1) on error
        {
          printf("[KDBG] Error in execv()\r\n") ;
          printf("[KDBG] errno = %d, %s\r\n",errno,strerror(errno)) ;
          printf("[KDBG] returning false from do_exec() at line %d (#1)\r\n",__LINE__) ;
        //  return false ;
        }
        printf("[KDBG] (fork_pid = %d) returning false from do_exec() at line %d (#2)\r\n",fork_pid,__LINE__) ;
        child_failed_flag = true ;
        return_value = false ;
        return false ; // Should not be here, should not have returned from execv() if it worked
      }
      break ;
    default : // Parent
      {
        int status ;
        //waitpid( fork_pid, &status, 0 ) ;
        wait( &status ) ;
        printf("[KDBG] status returned by waitpid() = %d\r\n",status) ;
        if( status != 0 )
        {
          printf("[KDBG] returning false from do_exec() at line %d due to status != 0\r\n",__LINE__) ;
          return_value = false ;
          return false ;
        }
      }
      break ;
  }

  va_end(args);

  if( child_failed_flag )
  {
    printf("[KDBG] returning false from do_exec() at line %d (#6) due to check_failed_flag\r\n",__LINE__) ;
    return false ;
  }

  printf("[KDBG] (fork_pid = %d) returning %s from do_exec() at line %d\r\n",fork_pid,((return_value)?("true"):("false")),__LINE__) ;

  return return_value ;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
  printf("[KDBG] Entering function do_exec_redirect()\r\n") ;

  va_list args;
  va_start(args, count);
  char * command[count+1];
  int i;
  for(i=0; i<count; i++)
  {
    command[i] = va_arg(args, char *);
    printf("[KDBG] command[%d] = '%s'\r\n",i,command[i]) ;
  }
  command[count] = NULL;


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

  int kidpid ;
  printf("[KDBG] Redirecting output filename '%s'\r\n",outputfile) ;
  int fd = open( outputfile , O_WRONLY|O_TRUNC|O_CREAT, 0644 ) ;
  printf("[KDBG] fp = %d\r\n",fd) ;
  if( fd < 0 )
  {
    printf("[KDBG] Could not open redirect file '%s'\r\n",outputfile) ;
    perror("open") ;
    return false ; // abort();
  }
  fflush(stdout) ;
  kidpid = fork() ;
  printf("[KDBG] kidpid = %d\r\n",kidpid) ;
  switch( kidpid )
  {
    case -1 :
      printf("[KDBG] fork() failed, returned %d\r\n",kidpid) ;
      perror("fork") ;
      return false ; // abort();
      break ;
    case 0: // Child
      if( dup2(fd, 1) < 0 )
      {
        perror("dup2") ;
        return false ; // abort();
      }
      close(fd);
      execvp( command[0] , command ) ; // Should not return from this
      perror("execvp") ;
      return false ;
      break ; //  abort();
    default: // Parent
      close(fd);
      int status ;
      waitpid( kidpid, &status, 0 ) ;
      printf("[KDBG] status returned by waitpid() = %d\r\n",status) ;
      if( status != 0 )
        return false ;
      break ;
  }

    va_end(args);

    return true;
}
