/*
 * Copyright: JessMA Open Source (ldcsaa@gmail.com)
 *
 * Author	: Bruce Liang
 * Website	: https://github.com/ldcsaa
 * Project	: https://github.com/ldcsaa/HP-Socket
 * Blog		: http://www.cnblogs.com/ldcsaa
 * Wiki		: http://www.oschina.net/p/hp-socket
 * QQ Group	: 44636872, 75375912
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "HPSocket4C.h"

#ifdef _SSL_SUPPORT

/************************************************************************
���ƣ����� SSL Socket ����ָ�����ͱ���
�������� SSL Socket ����ָ�붨��Ϊ��ֱ�۵ı���
************************************************************************/

typedef HP_Object	HP_SSLServer;
typedef HP_Object	HP_SSLAgent;
typedef HP_Object	HP_SSLClient;
typedef HP_Object	HP_SSLPullServer;
typedef HP_Object	HP_SSLPullAgent;
typedef HP_Object	HP_SSLPullClient;
typedef HP_Object	HP_SSLPackServer;
typedef HP_Object	HP_SSLPackAgent;
typedef HP_Object	HP_SSLPackClient;

typedef HP_Object	HP_HttpsServer;
typedef HP_Object	HP_HttpsAgent;
typedef HP_Object	HP_HttpsClient;
typedef HP_Object	HP_HttpsSyncClient;

/*****************************************************************************************************************************************************/
/******************************************************************** SSL Exports ********************************************************************/
/*****************************************************************************************************************************************************/

/********************************************************/
/************** HPSocket4C-SSL ���󴴽����� **************/

// ���� HP_SSLServer ����
HPSOCKET_API HP_SSLServer __HP_CALL Create_HP_SSLServer(HP_TcpServerListener pListener);
// ���� HP_SSLAgent ����
HPSOCKET_API HP_SSLAgent __HP_CALL Create_HP_SSLAgent(HP_TcpAgentListener pListener);
// ���� HP_SSLClient ����
HPSOCKET_API HP_SSLClient __HP_CALL Create_HP_SSLClient(HP_TcpClientListener pListener);
// ���� HP_SSLPullServer ����
HPSOCKET_API HP_SSLPullServer __HP_CALL Create_HP_SSLPullServer(HP_TcpPullServerListener pListener);
// ���� HP_SSLPullAgent ����
HPSOCKET_API HP_SSLPullAgent __HP_CALL Create_HP_SSLPullAgent(HP_TcpPullAgentListener pListener);
// ���� HP_SSLPullClient ����
HPSOCKET_API HP_SSLPullClient __HP_CALL Create_HP_SSLPullClient(HP_TcpPullClientListener pListener);
// ���� HP_SSLPackServer ����
HPSOCKET_API HP_SSLPackServer __HP_CALL Create_HP_SSLPackServer(HP_TcpServerListener pListener);
// ���� HP_SSLPackAgent ����
HPSOCKET_API HP_SSLPackAgent __HP_CALL Create_HP_SSLPackAgent(HP_TcpAgentListener pListener);
// ���� HP_SSLPackClient ����
HPSOCKET_API HP_SSLPackClient __HP_CALL Create_HP_SSLPackClient(HP_TcpClientListener pListener);

// ���� HP_SSLServer ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLServer(HP_SSLServer pServer);
// ���� HP_SSLAgent ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLAgent(HP_SSLAgent pAgent);
// ���� HP_SSLClient ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLClient(HP_SSLClient pClient);
// ���� HP_SSLPullServer ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLPullServer(HP_SSLPullServer pServer);
// ���� HP_SSLPullAgent ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLPullAgent(HP_SSLPullAgent pAgent);
// ���� HP_SSLPullClient ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLPullClient(HP_SSLPullClient pClient);
// ���� HP_SSLPackServer ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLPackServer(HP_SSLPackServer pServer);
// ���� HP_SSLPackAgent ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLPackAgent(HP_SSLPackAgent pAgent);
// ���� HP_SSLPackClient ����
HPSOCKET_API void __HP_CALL Destroy_HP_SSLPackClient(HP_SSLPackClient pClient);

/*****************************************************************************************************************************************************/
/*************************************************************** Global Function Exports *************************************************************/
/*****************************************************************************************************************************************************/

/***************************************************************************************/
/************************************ SSL ��ʼ������ ************************************/

/*
* ���ƣ�SNI Ĭ�ϻص�����
* ������HP_SSLServer_SetupSSLContext �����������ָ�� SNI �ص�������ʹ�ô� SNI Ĭ�ϻص�����
*		
* ������		lpszServerName	-- ��������
*			pContext		-- SSL Context ����
* 
* ����ֵ��SNI ����֤���Ӧ������
*/
HPSOCKET_API int __HP_CALL HP_SSL_DefaultServerNameCallback(LPCTSTR lpszServerName, PVOID pContext);

/*
* ���ƣ������ֲ߳̾����� SSL ��Դ
* �������κ�һ������ SSL ���̣߳�ͨ�Ž���ʱ����Ҫ�����ֲ߳̾����� SSL ��Դ
*		1�����̺߳� HP-Socket �����߳���ͨ�Ž���ʱ���Զ������ֲ߳̾����� SSL ��Դ����ˣ�һ������²����ֹ����ñ�����
*		2����������£����Զ����̲߳��� HP-Socket ͨ�Ų�������鵽 SSL �ڴ�й©ʱ������ÿ��ͨ�Ž���ʱ�Զ����̵߳��ñ�����
*		
* ������		dwThreadID	-- �߳� ID��0����ǰ�̣߳�
* 
* ����ֵ����
*/
HPSOCKET_API void __HP_CALL HP_SSL_RemoveThreadLocalState(DWORD dwThreadID);

/*
* ���ƣ���ʼ��ͨ����� SSL ��������
* ������SSL �������������� SSL ͨ���������ǰ��ɳ�ʼ������������ʧ��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCertFile			-- ֤���ļ�
*			lpszPemKeyFile			-- ˽Կ�ļ�
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCertFileOrPath	-- CA ֤���ļ���Ŀ¼��������֤��ͻ��˿�ѡ��
*			fnServerNameCallback	-- SNI �ص�����ָ�루��ѡ�����Ϊ nullptr ��ʹ�� SNI Ĭ�ϻص�������
*
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLServer_SetupSSLContext(HP_SSLServer pServer, int iVerifyMode /* SSL_VM_NONE */, LPCTSTR lpszPemCertFile /* nullptr */, LPCTSTR lpszPemKeyFile /* nullptr */, LPCTSTR lpszKeyPassword /* nullptr */, LPCTSTR lpszCAPemCertFileOrPath /* nullptr */, HP_Fn_SNI_ServerNameCallback fnServerNameCallback /* nullptr */);

/*
* ���ƣ���ʼ��ͨ����� SSL ����������ͨ���ڴ����֤�飩
* ������SSL �������������� SSL ͨ���������ǰ��ɳ�ʼ������������ʧ��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCert				-- ֤������
*			lpszPemKey				-- ˽Կ����
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCert			-- CA ֤�����ݣ�������֤��ͻ��˿�ѡ��
*			fnServerNameCallback	-- SNI �ص�����ָ�루��ѡ�����Ϊ nullptr ��ʹ�� SNI Ĭ�ϻص�������
*
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLServer_SetupSSLContextByMemory(HP_SSLServer pServer, int iVerifyMode /* SSL_VM_NONE */, LPCSTR lpszPemCert /* nullptr */, LPCSTR lpszPemKey /* nullptr */, LPCSTR lpszKeyPassword /* nullptr */, LPCSTR lpszCAPemCert /* nullptr */, HP_Fn_SNI_ServerNameCallback fnServerNameCallback /* nullptr */);

/*
* ���ƣ����� SNI ����֤��
* ������SSL ������� SetupSSLContext() �ɹ�����Ե��ñ��������Ӷ�� SNI ����֤��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCertFile			-- ֤���ļ�
*			lpszPemKeyFile			-- ˽Կ�ļ�
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCertFileOrPath	-- CA ֤���ļ���Ŀ¼��������֤��ѡ��
*
* ����ֵ��	����		-- �ɹ��������� SNI ����֤���Ӧ�������������������� SNI �ص������ж�λ SNI ����
*			����		-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API int __HP_CALL HP_SSLServer_AddSSLContext(HP_SSLServer pServer, int iVerifyMode, LPCTSTR lpszPemCertFile, LPCTSTR lpszPemKeyFile, LPCTSTR lpszKeyPassword /* nullptr */, LPCTSTR lpszCAPemCertFileOrPath /* nullptr */);

/*
* ���ƣ����� SNI ����֤�飨ͨ���ڴ����֤�飩
* ������SSL ������� SetupSSLContext() �ɹ�����Ե��ñ��������Ӷ�� SNI ����֤��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCert				-- ֤������
*			lpszPemKey				-- ˽Կ����
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCert			-- CA ֤�����ݣ�������֤��ѡ��
*
* ����ֵ��	����		-- �ɹ��������� SNI ����֤���Ӧ�������������������� SNI �ص������ж�λ SNI ����
*			����		-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API int __HP_CALL HP_SSLServer_AddSSLContextByMemory(HP_SSLServer pServer, int iVerifyMode, LPCSTR lpszPemCert, LPCSTR lpszPemKey, LPCSTR lpszKeyPassword /* nullptr */, LPCSTR lpszCAPemCert /* nullptr */);

/*
* ���ƣ��� SNI ��������
* ������SSL ������� AddSSLContext() �ɹ�����Ե��ñ����������������� SNI ����֤��
*		
* ������		lpszServerName		-- ��������
*			iContextIndex		-- SNI ����֤���Ӧ������
*
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLServer_BindSSLServerName(HP_SSLServer pServer, LPCTSTR lpszServerName, int iContextIndex);

/*
* ���ƣ�����ͨ����� SSL ���л���
* ����������ͨ����� SSL ���л��������� SSL ����ڴ�
*		1��ͨ���������ʱ���Զ����ñ�����
*		2����Ҫ��������ͨ����� SSL ��������ʱ����Ҫ�ȵ��ñ���������ԭ�ȵĻ�������
*		
* ������	��
* 
* ����ֵ����
*/
HPSOCKET_API void __HP_CALL HP_SSLServer_CleanupSSLContext(HP_SSLServer pServer);

/*
* ���ƣ���ʼ��ͨ����� SSL ��������
* ������SSL �������������� SSL ͨ���������ǰ��ɳ�ʼ������������ʧ��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCertFile			-- ֤���ļ����ͻ��˿�ѡ��
*			lpszPemKeyFile			-- ˽Կ�ļ����ͻ��˿�ѡ��
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCertFileOrPath	-- CA ֤���ļ���Ŀ¼��������֤��ͻ��˿�ѡ��
*
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLAgent_SetupSSLContext(HP_SSLAgent pAgent, int iVerifyMode /* SSL_VM_NONE */, LPCTSTR lpszPemCertFile /* nullptr */, LPCTSTR lpszPemKeyFile /* nullptr */, LPCTSTR lpszKeyPassword /* nullptr */, LPCTSTR lpszCAPemCertFileOrPath /* nullptr */);

/*
* ���ƣ���ʼ��ͨ����� SSL ����������ͨ���ڴ����֤�飩
* ������SSL �������������� SSL ͨ���������ǰ��ɳ�ʼ������������ʧ��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCert				-- ֤������
*			lpszPemKey				-- ˽Կ����
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCert			-- CA ֤�����ݣ�������֤��ͻ��˿�ѡ��
*
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLAgent_SetupSSLContextByMemory(HP_SSLAgent pAgent, int iVerifyMode /* SSL_VM_NONE */, LPCSTR lpszPemCert /* nullptr */, LPCSTR lpszPemKey /* nullptr */, LPCSTR lpszKeyPassword /* nullptr */, LPCSTR lpszCAPemCert /* nullptr */);

/*
* ���ƣ�����ͨ����� SSL ���л���
* ����������ͨ����� SSL ���л��������� SSL ����ڴ�
*		1��ͨ���������ʱ���Զ����ñ�����
*		2����Ҫ��������ͨ����� SSL ��������ʱ����Ҫ�ȵ��ñ���������ԭ�ȵĻ�������
*		
* ������	��
* 
* ����ֵ����
*/
HPSOCKET_API void __HP_CALL HP_SSLAgent_CleanupSSLContext(HP_SSLAgent pAgent);

/*
* ���ƣ���ʼ��ͨ����� SSL ��������
* ������SSL �������������� SSL ͨ���������ǰ��ɳ�ʼ������������ʧ��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCertFile			-- ֤���ļ����ͻ��˿�ѡ��
*			lpszPemKeyFile			-- ˽Կ�ļ����ͻ��˿�ѡ��
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCertFileOrPath	-- CA ֤���ļ���Ŀ¼��������֤��ͻ��˿�ѡ��
*
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLClient_SetupSSLContext(HP_SSLClient pClient, int iVerifyMode /* SSL_VM_NONE */, LPCTSTR lpszPemCertFile /* nullptr */, LPCTSTR lpszPemKeyFile /* nullptr */, LPCTSTR lpszKeyPassword /* nullptr */, LPCTSTR lpszCAPemCertFileOrPath /* nullptr */);

/*
* ���ƣ���ʼ��ͨ����� SSL ����������ͨ���ڴ����֤�飩
* ������SSL �������������� SSL ͨ���������ǰ��ɳ�ʼ������������ʧ��
*		
* ������		iVerifyMode				-- SSL ��֤ģʽ���ο� EnSSLVerifyMode��
*			lpszPemCert				-- ֤������
*			lpszPemKey				-- ˽Կ����
*			lpszKeyPassword			-- ˽Կ���루û��������Ϊ�գ�
*			lpszCAPemCert			-- CA ֤�����ݣ�������֤��ͻ��˿�ѡ��
*
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLClient_SetupSSLContextByMemory(HP_SSLClient pClient, int iVerifyMode /* SSL_VM_NONE */, LPCSTR lpszPemCert /* nullptr */, LPCSTR lpszPemKey /* nullptr */, LPCSTR lpszKeyPassword /* nullptr */, LPCSTR lpszCAPemCert /* nullptr */);

/*
* ���ƣ�����ͨ����� SSL ���л���
* ����������ͨ����� SSL ���л��������� SSL ����ڴ�
*		1��ͨ���������ʱ���Զ����ñ�����
*		2����Ҫ��������ͨ����� SSL ��������ʱ����Ҫ�ȵ��ñ���������ԭ�ȵĻ�������
*		
* ������	��
* 
* ����ֵ����
*/
HPSOCKET_API void __HP_CALL HP_SSLClient_CleanupSSLContext(HP_SSLClient pClient);

/***************************************************************************************/
/************************************* SSL �������� ************************************/

/*
* ���ƣ����� SSL ����
* ��������ͨ���������Ϊ���Զ�����ʱ����Ҫ���ñ��������� SSL ����
*		
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLServer_StartSSLHandShake(HP_SSLServer pServer, HP_CONNID dwConnID);

/* ����ͨ��������ַ�ʽ��Ĭ�ϣ�TRUE���Զ����֣� */
HPSOCKET_API void __HP_CALL HP_SSLServer_SetSSLAutoHandShake(HP_SSLServer pServer, BOOL bAutoHandShake);
/* ��ȡͨ��������ַ�ʽ */
HPSOCKET_API BOOL __HP_CALL HP_SSLServer_IsSSLAutoHandShake(HP_SSLServer pServer);

/* ���� SSL �����㷨�б� */
HPSOCKET_API void __HP_CALL HP_SSLServer_SetSSLCipherList(HP_SSLServer pServer, LPCTSTR lpszCipherList);
/* ��ȡ SSL �����㷨�б� */
HPSOCKET_API LPCTSTR __HP_CALL HP_SSLServer_GetSSLCipherList(HP_SSLServer pServer);

/*
* ���ƣ���ȡ SSL Session ��Ϣ
* ��������ȡָ�����͵� SSL Session ��Ϣ��������Ͳο���En_HP_SSLSessionInfo��
*		
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLServer_GetSSLSessionInfo(HP_SSLServer pServer, HP_CONNID dwConnID, En_HP_SSLSessionInfo enInfo, LPVOID* lppInfo);

/*
* ���ƣ����� SSL ����
* ��������ͨ���������Ϊ���Զ�����ʱ����Ҫ���ñ��������� SSL ����
*		
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLAgent_StartSSLHandShake(HP_SSLAgent pAgent, HP_CONNID dwConnID);

/* ����ͨ��������ַ�ʽ��Ĭ�ϣ�TRUE���Զ����֣� */
HPSOCKET_API void __HP_CALL HP_SSLAgent_SetSSLAutoHandShake(HP_SSLAgent pAgent, BOOL bAutoHandShake);
/* ��ȡͨ��������ַ�ʽ */
HPSOCKET_API BOOL __HP_CALL HP_SSLAgent_IsSSLAutoHandShake(HP_SSLAgent pAgent);

/* ���� SSL �����㷨�б� */
HPSOCKET_API void __HP_CALL HP_SSLAgent_SetSSLCipherList(HP_SSLAgent pAgent, LPCTSTR lpszCipherList);
/* ��ȡ SSL �����㷨�б� */
HPSOCKET_API LPCTSTR __HP_CALL HP_SSLAgent_GetSSLCipherList(HP_SSLAgent pAgent);

/*
* ���ƣ���ȡ SSL Session ��Ϣ
* ��������ȡָ�����͵� SSL Session ��Ϣ��������Ͳο���En_HP_SSLSessionInfo��
*		
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLAgent_GetSSLSessionInfo(HP_SSLAgent pAgent, HP_CONNID dwConnID, En_HP_SSLSessionInfo enInfo, LPVOID* lppInfo);

/*
* ���ƣ����� SSL ����
* ��������ͨ���������Ϊ���Զ�����ʱ����Ҫ���ñ��������� SSL ����
*		
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLClient_StartSSLHandShake(HP_SSLClient pClient);

/* ����ͨ��������ַ�ʽ��Ĭ�ϣ�TRUE���Զ����֣� */
HPSOCKET_API void __HP_CALL HP_SSLClient_SetSSLAutoHandShake(HP_SSLClient pClient, BOOL bAutoHandShake);
/* ��ȡͨ��������ַ�ʽ */
HPSOCKET_API BOOL __HP_CALL HP_SSLClient_IsSSLAutoHandShake(HP_SSLClient pClient);

/* ���� SSL �����㷨�б� */
HPSOCKET_API void __HP_CALL HP_SSLClient_SetSSLCipherList(HP_SSLClient pClient, LPCTSTR lpszCipherList);
/* ��ȡ SSL �����㷨�б� */
HPSOCKET_API LPCTSTR __HP_CALL HP_SSLClient_GetSSLCipherList(HP_SSLClient pClient);

/*
* ���ƣ���ȡ SSL Session ��Ϣ
* ��������ȡָ�����͵� SSL Session ��Ϣ��������Ͳο���En_HP_SSLSessionInfo��
*		
* ����ֵ��	TRUE	-- �ɹ�
*			FALSE	-- ʧ�ܣ���ͨ�� SYS_GetLastError() ��ȡʧ��ԭ��
*/
HPSOCKET_API BOOL __HP_CALL HP_SSLClient_GetSSLSessionInfo(HP_SSLClient pClient, En_HP_SSLSessionInfo enInfo, LPVOID* lppInfo);

/*****************************************************************************************************************************************************/
/******************************************************************** HTTPS Exports ******************************************************************/
/*****************************************************************************************************************************************************/

#ifdef _HTTP_SUPPORT

/****************************************************/
/**************** HTTPS ���󴴽����� *****************/

// ���� HP_HttpsServer ����
HPSOCKET_API HP_HttpsServer __HP_CALL Create_HP_HttpsServer(HP_HttpServerListener pListener);
// ���� HP_HttpsAgent ����
HPSOCKET_API HP_HttpsAgent __HP_CALL Create_HP_HttpsAgent(HP_HttpAgentListener pListener);
// ���� HP_HttpsClient ����
HPSOCKET_API HP_HttpsClient __HP_CALL Create_HP_HttpsClient(HP_HttpClientListener pListener);
// ���� HP_HttpsSyncClient ����
HPSOCKET_API HP_HttpsSyncClient __HP_CALL Create_HP_HttpsSyncClient(HP_HttpClientListener pListener);

// ���� HP_HttpsServer ����
HPSOCKET_API void __HP_CALL Destroy_HP_HttpsServer(HP_HttpsServer pServer);
// ���� HP_HttpsAgent ����
HPSOCKET_API void __HP_CALL Destroy_HP_HttpsAgent(HP_HttpsAgent pAgent);
// ���� HP_HttpsClient ����
HPSOCKET_API void __HP_CALL Destroy_HP_HttpsClient(HP_HttpsClient pClient);
// ���� HP_HttpsSyncClient ����
HPSOCKET_API void __HP_CALL Destroy_HP_HttpsSyncClient(HP_HttpsSyncClient pClient);

#endif

#endif