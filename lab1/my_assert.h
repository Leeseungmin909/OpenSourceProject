#undef assert

// NDEBUG가 정의되었다면 assert() 함수가 링크되어 실행하는것을 차단해야함.
#ifdef NDEBUG
	#define my_assert(condition)((void)0)

#else 
	#define my_assert(condition) \
		do{ \
			if (!(condition)) { \
				fprintf(stderr, "Assertion 실패: %s, file %s, line %d\n", \
					#condition, __FILE__, __LINE__); \
				abort(); \
			} \
		} while(0)
#endif
