# lemSTL

## 0. 其它

### 0.1. 术语对应

- project 工程
- compile 编译: 编译当前文件
- build 构建, make 生成: 编译工程中当前未编译过、被修改后未编译过的所有源文件
- build all 全部构建: 编译工程中的所有源文件
- link 链接: 将编译后的源代码与所需库代码组合为可执行文件

### 0.2. 语法规则

- `const` applies to the thing left of it. If there is nothing on the left then it applies to the thing right of it. 
