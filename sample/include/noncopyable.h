#ifndef NONCOPY_H
#define NONCOPY_H


/*
noncopyable被继承后派生类对象可以正常的构造与析构，但是无法进行拷贝与赋值操作
因为派生类构造前一定要先调用基类的构造

但是当派生类指定了构造函数也可以构造

class A {
protected:
	A(const A&) = delete;
	A& operator=(const A&) = delete;
public:
	A() = default;
	~A() = default;
};

class B : A {
public:
	B() {}
	B(const B&):A() {}  // 这里指定了B的拷贝构造去调用A的默认构造
};

int main() {
	B b1;
	B b2(b1);
	return 0;
}
*/

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif