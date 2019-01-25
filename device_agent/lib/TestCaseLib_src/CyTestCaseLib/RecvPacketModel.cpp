#include "StdAfx.h"
#include ".\recvpacketmodel.h"
#include "MessageFormat.h"
#include <WinSock2.h>


CRecvPacketModel::CRecvPacketModel(void)
{
	m_pClassLog = NULL;
	m_IsNothing = TRUE;
	TRACE("��ʼ��\n");
}

CRecvPacketModel::~CRecvPacketModel(void)
{
}

int CRecvPacketModel::SetRecvBuffer(char* pBuffer,int nBufLen)
{
	CString strInfo("");
	char *pCur = pBuffer;
	int nCurLen = nBufLen; //���µĳ���

	if(m_IsNothing)// ��һ��
	{	
		if(pCur[0] == 0x5a) // У��
		{
			Log(LOGDEBUG,"@�յ�0x5a");
			m_IsNothing = FALSE; // ��ʼ�հ�
			pCur = pCur+1;
			nCurLen = nCurLen -1;
		}
		else
		{
			strInfo.Format("@���������⣨��Ϣ��%d������Ҫ��Ϣ%d��\n",m_nMsgLen,m_nBuffLen);
			Log(LOGDEBUG,strInfo);
			return 0;
		}

		if(nCurLen == 0) // ֻ��0x5a
		{
			Log(LOGDEBUG,"@ֻ��0x5a");
			return 0;
		}
		else
		{
			Log(LOGDEBUG,"@������Ϣͷ");
		}
		
		if(nCurLen >= sizeof(UAGENT_MSG_HEAD))
		{
			UAGENT_MSG_HEAD *pMsg = (UAGENT_MSG_HEAD*)pCur;
			m_nMsgLen = ntohl(pMsg->u32MsgSize) + sizeof(UAGENT_MSG_HEAD);

			if(nCurLen >= m_nMsgLen)//������Ϣ������
			{
				strInfo.Format("@��һ����������Ϣ������ Data%d Msg%d",nCurLen,m_nMsgLen);
				Log(LOGDEBUG,strInfo);
				if(nCurLen>m_nMsgLen)
				{
					strInfo.Format("@��һ����Ϣ����%d",nCurLen - m_nMsgLen);
					Log(LOGDEBUG,strInfo);
				}
				memcpy(m_Buffer,pCur,m_nMsgLen);
				m_nBuffLen = m_nMsgLen;
				return (nCurLen-m_nMsgLen); // ʣ�������һ����Ϣ��
			}
			else //ֻ�ǵ�һ��
			{
				strInfo.Format("@ֻ�ǵ�һ������Ϣû���� Data%d Msg%d ����Ҫ%d\n",nCurLen,m_nMsgLen,m_nMsgLen-nCurLen);
				Log(LOGDEBUG,strInfo);
				memcpy(m_Buffer,pCur,nCurLen);
				m_nBuffLen = nCurLen;
				return 0;
			}
		}
		else // ʣ�����ݻ�û����Ϣͷ��
		{
			Log(LOGDEBUG,"�����㳤��");
			return 0;
		}
	}
	else // ���ǵ�һ��
	{
		// ����0x5a�Ѿ��չ�
		if(0 == m_nMsgLen) // ��Ҫ��ͷ
		{
			Log(LOGDEBUG,"@����Ϣͷ");
			UAGENT_MSG_HEAD *pMsg = (UAGENT_MSG_HEAD*)pBuffer;
			m_nMsgLen = ntohl(pMsg->u32MsgSize) + sizeof(UAGENT_MSG_HEAD);

			if(nCurLen >= m_nMsgLen)//������Ϣ������
			{
				strInfo.Format("@��һ����������Ϣ������ Data%d Msg%d ,����ԣ%d",nCurLen,m_nMsgLen,nCurLen - m_nMsgLen);
				Log(LOGDEBUG,strInfo);
				memcpy(m_Buffer,pCur,m_nMsgLen);
				m_nBuffLen = m_nMsgLen;
				return (nCurLen-m_nMsgLen); // ʣ�������һ����Ϣ��
			}
			else // ֻ��һ����
			{
				strInfo.Format("@ֻ�ǵ�һ������Ϣû���� Data%d Msg%d ����Ҫ%d",nCurLen,m_nMsgLen,m_nMsgLen-nCurLen);
				Log(LOGDEBUG,strInfo);
				memcpy(m_Buffer,pCur,nCurLen);
				m_nBuffLen = nCurLen;
				return 0;

			}
		}
		else // ��Ϣͷ�Ѿ��չ�
		{
			Log(LOGDEBUG,"@��Ϣͷ�Ѿ��չ�");
			if(nCurLen >= GetLackLen())//�Ѿ����Բ���
			{
				int nRichDataLen = nCurLen - GetLackLen();
				strInfo.Format("@��Ϣ�����һ��,����ԣ%d\n",nRichDataLen);
				Log(LOGDEBUG,strInfo);
				memcpy(m_Buffer + m_nBuffLen,pCur,GetLackLen());
				m_nBuffLen = m_nMsgLen;
				//return (nCurLen - GetLackLen()); //GetLackLen�Ǳ仯�ģ������Ҹ�ֵ�ĸı䣬�˴��ڱ仯 BUG_15:44���
				return nRichDataLen;
			}
			else // �����ܲ��㣬��Ҫ��������
			{
				strInfo.Format("@��Ϣ���м�һ��,����%d\n",GetLackLen()- nCurLen);
				Log(LOGDEBUG,strInfo);
				memcpy(m_Buffer + m_nBuffLen,pCur,nCurLen);
				m_nBuffLen = m_nBuffLen + nCurLen;
				return 0;
			}
		}
	}
	return 0;
}

void CRecvPacketModel::ResetModel()
{
	m_IsNothing = TRUE;
	m_nMsgLen = 0;
	m_nBuffLen = 0;
	memset(m_Buffer,0,BUFFER_DATA_RECV_LEN);
}


BOOL CRecvPacketModel::IsIntegrityMsg()
{
	if((m_nMsgLen == m_nBuffLen) && (m_nBuffLen!=0) )
	{
		return TRUE;
	}
	return FALSE;
}

int CRecvPacketModel::GetMsgLen()
{
	return m_nMsgLen;
}

int CRecvPacketModel::GetLackLen()
{
	return (m_nMsgLen - m_nBuffLen);
}


void CRecvPacketModel::Log(int n,CString strInfo)
{
	//��λ��������ע��
	//if(m_pClassLog != NULL)
	//{
	//	fprintf(m_pClassLog,strInfo);
	//	fprintf(m_pClassLog,"\n");
	//	fflush(m_pClassLog);
	//}
}