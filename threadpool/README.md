
半同步/半反应堆线程池
===============
使用一个工作队列完全解除了主线程和工作线程的耦合关系：主线程往工作队列中插入任务，工作线程通过竞争来取得任务并执行它。
> * 同步I/O模拟proactor模式
> * 半同步/半反应堆
> * 线程池


### 知识点
**RAII**：使用类来管理资源，将资源和对象的生命周期绑定（自动初始化，自动回收）

**static**：
1. 静态成员变量
   - 编译阶段就分配空间，这是在对象创建之前就完成了，位于全局静态区
   - 最好是类内声明，类外初始化（以免类名访问静态成员访问不到）。
2. 静态成员函数
   - 可直接访问静态成员变量，但是不能直接访问普通成员变量
   - 没有this指针。非静态数据成员为对象单独维护，但静态成员函数为共享函数，无法区分是哪个对象

**pthread_create**:
```cpp
#include <pthread.h>
int pthread_create (pthread_t *thread_tid,                //返回新生成的线程的id
                    const pthread_attr_t *attr,           //指向线程属性的指针,通常设置为NULL
                    void * (*start_routine) (void *),    //处理线程函数的地址
                    void *arg);                          //start_routine()中的参数
```
对于线程函数，由于接收的参数类型为一个`void*`, 如果处理的类成员函数，则这时候只能是静态类成员函数，因为类成员函数默认的第一个参数
为this指针。






