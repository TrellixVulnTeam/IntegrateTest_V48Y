#ifndef FUNCTION_HEADER
#define FUNCTION_HEADER

	#ifdef TESTCASE_DLL_EXPORTS 
		#define TESTCASE_DLL_API __declspec(dllexport)  
	#else  
		#define TESTCASE_DLL_API __declspec(dllimport)  
	#endif

// ������������-���ɿ��ޣ���ʵ���������
//extern "C" TESTCASE_DLL_API void TestCase_Init(void);

#endif //FUNCTION_HEADER