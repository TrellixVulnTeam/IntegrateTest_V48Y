#pragma once

#define BUFFER_DATA_RECV_LEN 1024*11 //BUG 20171123 ��Eeprom�쳣������Ϊ�˴�ԭ��Ϊ4096̫С�ˣ�Msgsize����ʱ������쳣���쳣��memcpy(m_Buffer,pCur,m_nMsgLen);
                                    // ��������ʵ�������ж϶�λֻ�ܿ���memcpy�����ڴ���������memcpy����ϵ㣬��������Խ��
                                    // �·�������������102400���ټ���ͷ�����Դ˴�����1024*11�����ٳ��� ��������Ӧ�üӳ����жϣ�
                            
// �����������Ҫ����
class CRecvPacketModel
{
public:
	CRecvPacketModel(void);
	~CRecvPacketModel(void);

	FILE *m_pClassLog;

	// ����������һ����Ϣ�ĳ��ȣ�ֻ�����Լ���0x5a,��һ���Ĳ��������أ�
	// Set֮��������صĲ���0���Ϳ���DealMsg��Ȼ�����ResetModel���ٵ���Set���д���
	//          ������ص���0���ͼ����Ϣ�Ƿ���������������Ϳ��Դ�����
	int SetRecvBuffer(char* pBuffer,int nBufLen);//���յ������ݷ�����д���

    void ResetModel(); //׼��������һ�������� 


	BOOL IsIntegrityMsg(); // �Ƿ�����������Ϣ������ǾͿ��Խ�����
	int GetMsgLen();   // ��ȡ��Ϣ����
	int GetLackLen();  // ��ȡ��ȱ�ٵĳ���


	BOOL m_IsNothing;// �Ƿ���ɶҲû��
 	char m_Buffer[BUFFER_DATA_RECV_LEN];// ��Ϣ����

private:
	int m_nMsgLen; // ��Ϣ����(����Ϣͷ)
	int m_nBuffLen;// ����Buffer�ĳ���
	void Log(int n,CString strInfo);


};
