#pragma once

// ���֡��
const int_x STACK_MAX_FRAME = 512;
// �������
const int_x STACK_MAX_FUNCTION = 1024;
// ����ļ�������
const int_x STACK_MAX_FILE = 512;

// һ��ջ֡
struct stackframe_t
{
public:
	char_8 function[STACK_MAX_FUNCTION];
	char_8 file[STACK_MAX_FILE];
	char_8 module[STACK_MAX_FILE];
	int_x line;
	int_x addr;
};

// �ɶ��ջ֡��ɵĶ�ջ
class callstack_t
{
public:
	vector<stackframe_t> frames;

	bool generate(const void * pContext = nullptr, int_x iSkip = 0);
};

bool symblehlp_initialize();
