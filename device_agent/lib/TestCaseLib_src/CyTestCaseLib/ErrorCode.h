#define TESTCASE_SUCCESS 0
#define TESTCASE_FAIL -1

#define INIT_SOCKET_FAIL -1
#define INT_LOG_FAIL -2
#define INT_OTHER_FAIL -3

#define REGISTER_CONNECT_FAIL -1
#define REGISTER_IPINUSE_FAIL -2
#define REGISTER_NAMEINUSE_FAIL -2


#define EXECUTECMD_SEND_FAIL -1
#define EXECUTECMD_PARA_TOOMANY -2


/* ������� */
/* ��Ч�������� */
#define ECMD_INVALID_ERROR                        -1
/* δע��İ忨���� */
#define ECMD_NOT_REGISTERED_ERROR                 -2
/* RSP1��Ӧ��ʱ */
#define ECMD_RSP1_TIMEOUT_ERROR                   -3
/* RSP1��Ӧ:����δ�ҵ� */
#define ECMD_RSP1_NO_SYMBOL_ERROR                 -4
/* RSP1��Ӧ:���Ų���ִ��, (��ʱ���ظ÷���ֵ) */
#define ECMD_RSP1_NONE_EXECUTE_SYMBOL_ERROR       -5
/* RSP2��Ӧ:��ʱ */
#define ECMD_RSP2_TIMEOUT_ERROR                   -6


/* ��������RSP��ʱ */
#define ECMD_CTRL_RSP_TIMEOUT_ERROR               -7
/* ��������ִ��ʧ�� */
#define ECMD_CTRL_RSP_FAIL                        -8
/* UAgtCmd����æ */
#define ECMD_TASK_BUSY                            -9
/* ����Buffer���� */
#define ECMD_CMD_BUF_FULL                         -10