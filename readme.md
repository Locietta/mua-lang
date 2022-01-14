# mua-lang

A simple lisp-like functional language's interpreter, mostly based on PPL(Principle of Programming Language) course assignment of ZJU.

一个简单的lisp-like函数式语言解释器，基于PPL的课程大作业。

## 实现的特性

- [x] 基本数据类型定义和运算
- [x] 逻辑运算和if语句分支
- [x] 列表操作
- [x] 闭包和高阶函数
- [x] 保存和加载命名空间（不含闭包）
- [ ] 保存和加载闭包
- [ ] 尾递归优化
- [ ] 注释
- [ ] 高精度数字

### 语法说明 Language Specification

详细的语法说明见[mua-spec.md](./mua-spec.md).

## mua代码示例

### 最大公约数

一个简单的递归求解最大公约数的例子

```
make "gcd [[a b][
    if eq :b 0
        [return :a]
        [return gcd :b mod :a :b]
]]
print gcd 18 14
print gcd 18 13
print gcd 120 12

# output: 2 1 12
```

### 使用Z组合子求阶乘

使用Z组合子来求解阶乘，注意大量使用了闭包

```
make "fact_nicer [[rec][
    make "g [[x][
        if eq :x 0
            [return 1]
            [return mul :x rec sub :x 1]
    ]]
    return :g
]]

make "z_comb [[g][
    make "t [[r][
        make "y [[yy][
            make "tmp r :r
            return tmp :yy
        ]]
        return g :y
    ]]
    return t :t
]]

make "fact_z z_comb :fact_nicer
print fact_z 5

# ouput: 120
```

## License

THIS PROJECT IS OPEN SOURCE UNDER MIT LICENSE, BUT WITH A LIMITATION:

**ANYONE CANNOT USE ANY PART OF THIS PROJECT IN ANY COURSE OF ZJU AS COURSE ASSIGNMENT.**

项目根据MIT协议开源，但有如下限制：

**任何人不得在浙江大学的任何课程中使用本项目的任何部分作为课程作业。**