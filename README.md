# mips-simulator
ACM Class 2017 Summer PPCA 大...作...业...


## 结构设计

### 存储
- 内存: 无需考虑内存回收
    - 数据区: 4G`char`数组模拟 地址即数组下标 小端序
        - 堆内存: 堆顶指针
        - 栈内存: 栈顶指针 `$sp`(30号寄存器`$29`)
    - 代码区: `vector<instruction*>`实现
        - 指令: `class instruction`关系由继承体现

- 寄存器: `int`变量模拟 `$0`-`$31`, `$lo`, `$hi`
- label: 指针模拟 变量/指令 `map<string, int>`实现
- 5级pipeline: 5个工作台 分别记录当前工作状态

### 运行
- 读一遍文件
    - `.data`部分存入内存区 label赋值
    - `.text`部分存入代码区 label赋值
- 顺序运行内存区代码
- 顺序运行代码区代码
    - 1个滴答(并行)处理一次流水线 从后往前处理
    
## 5级流水
- 串行: 一个周期内从WB到IF依次执行
- 需要处理的hazard
    - jump branch
    - 流水线上的指令会修改当前fetch的指令需要读取的寄存器
    - 当前周期MA需要写内存的时候IF不能读内存
- 关联的寄存器(修改/读取)用二进制数存在`instruction`的成员变量里
- 还需要记录jump/branch类型
