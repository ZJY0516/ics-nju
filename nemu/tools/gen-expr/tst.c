#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  // 打开文件（假设文件名为input.txt）
  FILE *file = fopen("input", "r");
  if (file == NULL) {
    perror("无法打开文件");
    return 1;
  }

  // 用于存储每行的内容
  char line[50000]; // 假设每行不超过 255 个字符

  while (fgets(line, sizeof(line), file)) {
    // 删除换行符（如果存在）
    size_t line_length = strlen(line);
    if (line_length > 0 && line[line_length - 1] == '\n') {
      line[line_length - 1] = '\0';
    }

    // 使用空格分割结果和表达式
    char *result_str = strtok(line, " ");
    char *expression_str = strtok(NULL, " ");

    if (result_str != NULL && expression_str != NULL) {
      // 打印结果和表达式
      printf("结果: %s\n", result_str);
      printf("表达式: %s\n", expression_str);
    }
  }

  // 关闭文件
  fclose(file);

  return 0;
}
