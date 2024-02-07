# 作业 7：路径追踪

## 背景知识

### 渲染方程

Whitted-Style 的不足：

1. 只支持完全镜面反射和折射，不支持“有点镜面但不完全镜面”的材质。
2. 光线遇到漫反射材质直接停止反弹，不符合能量守恒定律，表现在画面上就是没有 color bleeding。

为了渲染出符合物理规律的真实画面，需要用到渲染方程：

![image-20240207195244215](README/image-20240207195244215.png)

下面逐个解释方程中的符号，先看等号左边：

- p：光线打中的点的坐标。
- w_o：出射方向向量。（o=outgoing）
- L_o：出射光强度，可以理解为从 w_o 方向观测到点 p 的光强度。

（渲染方程中所有 L 都表示物理上的 radiance，不过这不太重要，粗略地当做“光的强度”就行）

等号右边由两部分组成，分别是“自发光”和“反射方程”。

- 自发光
  - L_e：物体在点 p、w_o 方向上的自发光强度。（e=emission）
- 反射方程
  - w_i：入射方向向量。（i=incident）
  - Ω+：因为四面八方的入射光都对反射光有一定的贡献，所以要对入射方向 w_i 积分。Ω+ 表示半球的积分范围。
  - L_i：入射光的强度，因为光线会不断反弹，所以 L_i 由其它物体的 L_o 组成，求 L_i 是一个递归的过程。
  - f_r：表示在点 p 从 w_i 方向入射的光有多少比例反射到了 w_o 方向。f_r 是一个“双向反射分布函数（BRDF）”，不同的材质有不同的反射分布规律，比如镜面材质的反射能量集中在与入射方向对称的方向，漫反射材质的反射能量均匀地分散在半球的各个方向。
  - (n·wi)：法向量和入射光向量夹角的余弦值。（Lambert 余弦定理：同样强度的光，斜着照比正着照更暗。）

### 光栅化与光追

其实光栅化就是只计算光线反弹 0 次（自发光）和 1 次（物体受到光照）的结果，而光追会使光线在物体之间反弹，这样就有了镜面反射、全局光照等效果。

### 蒙特卡洛积分

渲染方程的整体结构长这样：

```
出射光强度 = 自发光（由材质决定）+ 反射方程（一坨有点难算还带递归的积分）
```

为了在有限的时间内把那坨积分解出来，Path Tracing 采用了蒙特卡洛积分法。

![image-20240204120905496](README/image-20240204120905496.png)

随机取一个 x 求函数值，然后乘以积分区间的长度，得到一个长方形的面积。多取样几次，把算出来的长方形的面积求平均，就能蒙出函数积分的近似值。取样越多，蒙得越准。

![image-20240204120722257](README/image-20240204120722257.png)

这里的 p(x) 是概率分布函数，它隐含了积分区间的信息。

举个例子，求某 f(x) 在 0~5 的积分，那么 p(x) 就应该长这样：

![image-20240204130055353](README/image-20240204130055353.png)

这意味着 x 将在 0~5 均匀地取样，而取到 0~5 范围以外的值的概率为 0。

套上面的公式，不管取样在哪里，除以 p(x) 其实都是除以 0.2，也就是乘以区间长度 5。

所以 p(x) 隐含了区间的取样范围，并且因为均匀分布，p(x) 的倒数 = 取样范围长度。

### Path Tracing

路径追踪的初步伪代码：

```pseudocode
shade(p, wo)
    Randomly choose ONE directions w_i~pdf
    Trace a ray r(p, wi)
    If ray r hit the light
        Return (1 / N) * L_i * f_r * cosine / pdf(wi)
    Else If ray r hit an object at q
        Return (1 / N) * shade(q, -wi) * f_r * cosine / pdf(wi)
```

1. 蒙特卡洛积分：随机选取入射方向计算反射方程，除以 pdf。
2. 只随机取**一个**入射方向，因为如果取的方向数量更多，多递归几层就指数爆炸了。（如果取多个入射方向，就叫分布式光线追踪）

这个代码没有递归出口，如果光线一直在物体之间弹射就会陷入死循环，引入“俄罗斯轮盘赌”：

```pseudocode
shade(p, wo)
    Manually specify a probability P_RR
    Randomly select ksi in a uniform dist. in [0, 1]
    If (ksi > P_RR) return 0.0;
    
    Randomly choose ONE direction wi~pdf(w)
    Trace a ray r(p, wi)
    If ray r hit the light
        Return L_i * f_r * cosine / pdf(wi) / P_RR
    Else If ray r hit an object at q
        Return shade(q, -wi) * f_r * cosine / pdf(wi) / P_RR
```

每次递归都有 P_RR 的概率 Return 0，除以 P_RR，这样它的期望值仍然是 L_o。

目前为止代码已经能正确渲染了，但效率不够高，因为弹射到光源的概率比较低。



## 作业

