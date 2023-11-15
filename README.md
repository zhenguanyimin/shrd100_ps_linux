# shrd100_ps_linux

tracer嵌入式代码，Linux系统

CI:
1. 分支推送至gitlab和发起合并请求时，自动编译app
2. kernel仅在手动触发和推送标签的编译

手动触发方式：
1. 打开[链接](https://adasgitlab.autel.com/shrd/shrd100_ps_linux/-/pipelines)，或点击右侧CI/CD -> 流水线
2. 点击右上角蓝色图标**运行流水线**，选择分支
3. 输入变量名称 `VERSION` ，输入变量的值为版本号，如 `SHRD101L-V1.01.01`
4. 点击**运行流水线**确认触发
5. 等待编译完成，日志中会打印下载链接

推送标签触发：
1. 本地推送```git tag SHRD101L-V1.01.01 && git push origin SHRD101L-V1.01.01```
2. 或者网页上点击仓库 -> 标签，点击右上角**新建标签**
