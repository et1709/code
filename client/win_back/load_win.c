#include"registered.h"
#include<stdio.h>
#include<string.h>
int login_information(struct information *info)
{
	int number;
	int account;
	char sh, ch;

	while((ch = getchar() != '\n' && ch != EOF));
	printf("请输入您的账号\n");
	fgets(info->login_account, 6, stdin);
		
	//printf("%c", info->login_account);
	printf("请输入登陆密码(您有三次输入密码的机会!):\n");
	for(number = 3; number >= 0; number--)
	{
		system("stty -echo");
		while((ch = getchar() != '\n' && ch != EOF));
		fgets(info->login_password, 6, stdin);
		system("stty echo");

		if(strcmp(info->password, info->login_password) == 0)
		{
			//printf("密码正确!\n");
			break;
		}

		memset(&info->login_password, 0, sizeof(info ->login_password));
		printf("您还有%d次机会:\n", number);

	}
	
	return 0;
}
