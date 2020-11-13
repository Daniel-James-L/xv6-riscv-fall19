#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char cmd_buf[1024];
char *buf_get, *buf_rep;//传递指针
void run_cmd(char *cmd);
// 处理 "|" ，用 "\0"代换
char *rep(char *a, char b)
{
    while (*a != '\0' && *a != b)
        a++;
    if (*a == '\0')
        return 0;
    *a = '\0';
    return a + 1;
}

//处理格式空格
char *trim(char *a)
{
    char *c = a;
    while (*c)
        c++;
    while (*a == ' ')
        *(a++) = '\0';
    while (*(--c) == ' ');
    *(c + 1) = '\0';
    return a;
}
//创建文件描述符
void redirect(int k, int pd[])
{
    close(k);
    dup(pd[k]);
    close(pd[0]);
    close(pd[1]);
}
inline int print(int fd, char *str)
{

	return write(fd, str, strlen(str));

}
void runcmd_pipe()
{
    if (buf_get)
    {
        int pd[2];
        pipe(pd);
        if(!fork()){
            if(buf_rep) redirect(1, pd);
            run_cmd(buf_get);
		}
        else if(!fork()) {
            if(buf_rep) {
                redirect(0, pd);
                buf_get = buf_rep;//pipe
                buf_rep = rep(buf_get, '|');
                runcmd_pipe();
            }

        }
        close(pd[0]);
        close(pd[1]);
        wait(0);
        wait(0);

    }
    exit(0);

}
void run_cmd(char *cmd)
{
	char buf[32][32];
	char *order[32];
	int argc = 0;
	char *order_cpy[32];//传递命令
	int argc_cpy = 0;
	//处理空格
	cmd = trim(cmd);

	for (int i = 0; i < 32; i++)
		order[i] = buf[i];

	char *temp = buf[argc];
	int input_pos = 0, output_pos = 0;
	for (char *p = cmd; *p; p++)
	{
		if (*p == ' ' || *p == '\n')
		{
			*temp = '\0';
			argc++;
			temp = buf[argc];
		}

		else {//重定向处理
			if (*p == '<') {
				input_pos = argc + 1;
			}
			if (*p == '>') {
				output_pos = argc + 1;
			}
			*temp++ = *p;
		}
	}
	*temp = '\0';
	argc++;
	order[argc] = 0;
	if (input_pos) {
		close(0);
		open(order[input_pos], O_RDONLY);//输入重定向

	}
	if (output_pos) {
		close(1);
		open(order[output_pos], O_WRONLY | O_CREATE);//输出重定向
	}

	for (int pos = 0; pos < argc; pos++) {

		if (pos == input_pos - 1)
			pos += 2;
		if (pos == output_pos - 1)
			pos += 2;

		order_cpy[argc_cpy++] = order[pos];
	}
	order_cpy[argc_cpy] = 0;//argv[size-1]必须为0
	if (fork()) {
		wait(0);
	}
	else {
		exec(order_cpy[0], order_cpy);
	}

}
int main(int argc, char *argv[])
{
    while (1)
    {
        print(1, "@ ");
        memset(cmd_buf, 0, 1024);
        gets(cmd_buf, 1024);
        if (cmd_buf[0] == 0) // EOF
            exit(0);
        *strchr(cmd_buf, '\n') = '\0';
        if (fork())
        {
            wait(0);
        }
        else
        {
            buf_get = cmd_buf;
            buf_rep = rep(buf_get, '|');
            runcmd_pipe();
        }
    }
    exit(0);

}