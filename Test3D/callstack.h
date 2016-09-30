#pragma once

// 最大帧数
const int_x STACK_MAX_FRAME = 512;
// 函数名最长
const int_x STACK_MAX_FUNCTION = 1024;
// 最大文件名长度
const int_x STACK_MAX_FILE = 512;

// 一个栈帧
struct stackframe_t
{
public:
	char_8 function[STACK_MAX_FUNCTION];
	char_8 file[STACK_MAX_FILE];
	char_8 module[STACK_MAX_FILE];
	int_x line;
	int_x addr;
};

// 由多个栈帧组成的堆栈
class callstack_t
{
public:
	vector<stackframe_t> frames;

	bool generate(const void * pContext = nullptr, int_x iSkip = 0);
};

bool symblehlp_initialize();
