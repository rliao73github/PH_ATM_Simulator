#include <stdio.h>
#include <stdlib.h> //计算
#include <time.h>       //时间函数
void showMenu();                     
void deposit(float *balance);        
void withdraw(float *balance);       
void checkBalance(float balance);    
void showTime();                     
int main() {                         
    float balance = 0.0;             
    int choice;                      

    printf("=== 欢迎使用ATM模拟器 ===\n"); 
    showTime();   
    while(1) {                       
        showMenu();                  
        printf("请输入您的选择: ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("无效输入，请输入数字！\n");
            continue;
        }

        switch(choice) {             
            case 1:                  
                deposit(&balance);   
                break;
            case 2:
                withdraw(&balance);
                break;
            case 3:
                checkBalance(balance);
                break;
            case 4:
                printf("感谢使用ATM模拟器，再见！\n");
                exit(0);
            default:
                printf("无效的选择，请重新输入！\n");
        }

        printf("\n");
    }

    return 0;
}

void showMenu() {
    printf("\n=== ATM 菜单 ===\n");
    printf("1. 存款\n");
    printf("2. 取款\n");
    printf("3. 查询余额\n");
    printf("4. 退出\n");
}


void showTime() {
    time_t now;
    time(&now);

    struct tm *t = localtime(&now);

    printf("当前时间：%04d-%02d-%02d %02d:%02d:%02d\n",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec
    );
}

// 存款
void deposit(float *balance) {
    float amount;
    printf("请输入存款金额: ");

    if (scanf("%f", &amount) != 1) {
        printf("无效输入，请输入一个有效的金额！\n");
        while (getchar() != '\n');
        return;
    }

    if(amount <= 0) {
        printf("存款金额必须大于0！\n");
        return;
    }

    *balance += amount;

    // ⭐ 每次操作后显示时间
    printf("\n存款成功！操作时间：\n");
    showTime();

    printf("存款金额: %.2f 元\n", amount);
}

// 取款
void withdraw(float *balance) {
    float amount;

    if(*balance <= 0) {
        printf("余额不足，无法取款！\n");
        return;
    }

    printf("请输入取款金额: ");

    if (scanf("%f", &amount) != 1) {
        printf("无效输入，请输入一个有效的金额！\n");
        while (getchar() != '\n');
        return;
    }

    if(amount <= 0) {
        printf("取款金额必须大于0！\n");
        return;
    }

    if(amount > *balance) {
        printf("余额不足！当前余额: %.2f 元\n", *balance);
        return;
    }

    *balance -= amount;

    printf("\n取款成功！操作时间：\n");
    showTime();   
    printf("取款金额: %.2f 元\n", amount);
    printf("当前余额: %.2f 元\n", *balance);
}

// 查询余额
void checkBalance(float balance) {
    printf("=== 账户余额 ===\n");
    printf("当前余额: %.2f 元\n", balance);

    printf("查询时间：\n");
    showTime();   
}
