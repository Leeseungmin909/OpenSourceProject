# 사용할 컴파일러
CC = gcc

# 컴파일 옵션
CFLAGS = -Wall -I./lib

# 라이브러리로 만들 목적 파일 목록을 직접 지정
OBJS = lib/add.o lib/subtract.o lib/multiply.o lib/divide.o

# make 라고만 치면 lab2_static lab2_shared lab2_dynamic을 모두 만든다.
all: lab2_static lab2_shared lab2_dynamic

# 정적 라이브러리 방식
lab2_static: libTestMain.c lib/libtestlib.a
	$(CC) $(CFLAGS) libTestMain.c -L./lib -ltestlib -o lab2_static
lib/libtestlib.a: $(OBJS)
	ar rcs lib/libtestlib.a $(OBJS)

# 공유 라이브러리 방식
lab2_shared: libTestMain.c lib/libtestlib.so
	$(CC) $(CFLAGS) libTestMain.c -L./lib -ltestlib -o lab2_shared
lib/libtestlib.so: $(OBJS)
	$(CC) -fPIC -shared $(OBJS) -o lib/libtestlib.so

# 동적 라이브러리 방식
lab2_dynamic: libTestMainDlfcn.c
	$(CC) $(CFLAGS) libTestMainDlfcn.c -ldl -o lab2_dynamic

# lib 폴더의 .c 파일들을 .o 파일로 만드는 규칙
# $< 는 의존성 파일(lib/add.c 등), $@ 는 목표 파일(lib/add.o 등)을 의미
lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# make clean 실행 시 생성된 파일들 삭제
clean:
	rm -f lib/*.o 

