#ifndef CmdProcessor_h
#define CmdProcessor_h

#include <inttypes.h>

#if ARDUINO >= 100
#include "Arduino.h"  
#else
#include "WProgram.h"  
#endif


typedef void(*CmdHandler)(const char* args, void* data);

template<size_t MAX_CMDS, char cmdSep = ';', char argListSep = '?'>
class CmdProcessor
{
public:

  CmdProcessor()
  {
	memset(m_commands, 0, sizeof(m_commands));
  }

  ~CmdProcessor()
  {
  }

  bool registerCmd(const char* cmd, CmdHandler handler, void* data = NULL)
  {
	  for (int i=0; i<MAX_CMDS; i++)
	  {
		  if (!m_commands[i].cmd)
		  {
			  m_commands[i].cmd = cmd;
			  m_commands[i].handler = handler;
			  m_commands[i].data = data;
			  return true;
		  }
	  }
	  return false;

  }

  void unregisterCmd(const char* cmd)
  {
	  for (int i=0; i<MAX_CMDS; i++)
	  {
		  if (!m_commands[i].cmd)
			continue;

		  if (strcmp(m_commands[i].cmd, cmd) == 0)
		  {
			  m_commands[i].cmd = NULL;
			  //m_commands[i].handler = NULL;
			  //m_commands[i].data = NULL;
			  break;
		  }
	  }
  }

  bool process(const char* line)
  {
	  bool processed = false;

	  String cmdLine = line;

	  int start = 0;
	  while (start >= 0)
	  {
		  String cmd, args;

		  int end = cmdLine.indexOf(m_cmdSep, start);
		  if (end == -1)
		  {
			  cmd = cmdLine.substring(start);
			  start = -1;
		  }
		  else
		  {
			  cmd = cmdLine.substring(start, end);
			  start = end + 1;
		  }

		  int argsStart = cmd.indexOf(m_argListSep);
		  if (argsStart != -1)
		  {
			  args = cmd.substring(argsStart + 1);
			  cmd = cmd.substring(0, argsStart);
		  }

		  for (int i=0; i<MAX_CMDS; i++)
		  {
			  if (m_commands[i].cmd && cmd.equals(m_commands[i].cmd))
			  {
				  m_commands[i].handler(args.length() ? args.c_str() : NULL, m_commands[i].data);
				  processed = true;
				  break;
			  }
		  }
	  }
	  return processed;
  }

private:

  const char m_cmdSep = cmdSep;
  const char m_argListSep = argListSep;
 
  struct CmdDescriptor
  {
    const char* cmd;
    CmdHandler handler;
    void* data;
  };

  CmdDescriptor m_commands[MAX_CMDS];
};

#endif
