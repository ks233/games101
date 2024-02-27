//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"

void Scene::buildBVH()
{
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object *> &objects,
    float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear)
        {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    /*

    作业框架：

        intersect               光线求交
        sampleLight             采样一个光源，返回相交信息和概率
        sample(wi, N)           （材质属性）随机取一个出射向量
        pdf(wi, wo, N)          （材质属性）概率分布函数
        eval(wi, wo, N)         （材质属性）brdf
        RussianRoulette = 0.8   常数，轮盘赌概率

    伪代码：

    shade(p, wo)
        sampleLight(inter , pdf_light)
        Get x, ws, NN, emit from inter
        Shoot a ray from p to x
        If the ray is not blocked in the middle
        L_dir = emit * eval(wo, ws, N) * dot(ws, N) * dot(ws, NN) / |x-p|^2 / pdf_light

        L_indir = 0.0
        Test Russian Roulette with probability RussianRoulette
        wi = sample(wo, N)
        Trace a ray r(p, wi)
        If ray r hit a non-emitting object at q
            L_indir = shade(q, wi) * eval(wo, wi, N) * dot(wi, N) / pdf(wo, wi, N) / RussianRoulette

        Return L_dir + L_indir
    */
    Intersection inter = intersect(ray);

    if (!inter.happened)
        return Vector3f(); // 如果不直接返回，后面调用材质的方法会 segment fault

    // 没有这个 if 光源就不显示
    if (inter.m->hasEmission())
        return inter.m->getEmission();

    Vector3f x = inter.coords;
    Vector3f wo = ray.direction;
    Vector3f N = inter.normal;

    Intersection inter_L;
    float pdf_light;
    sampleLight(inter_L, pdf_light);
    Vector3f xx = inter_L.coords;
    Vector3f NN = inter_L.normal;
    Vector3f emit = inter_L.emit;
    Vector3f ws = (xx - x).normalized();

    Vector3f L_dir = Vector3f();

    // 创建一个光线计算直接光照遮挡
    // 起点要沿法向稍微出来一点点，否则直接和自己原地相交了
    Vector3f test_orig = (dotProduct(ray.direction, N) < 0) ? x + N * EPSILON : x - N * EPSILON;
    Intersection test = intersect(Ray(test_orig, ws));
    if ((test.coords - xx).norm() < 0.0001)
    {
        L_dir = emit * inter.m->eval(wo, ws, N) * dotProduct(ws, N) * dotProduct(-ws, NN) / dotProduct(xx - x, xx - x) / pdf_light;
    }

    if (get_random_float() > RussianRoulette)
        return L_dir;
    Vector3f wi = inter.m->sample(wo, N).normalized();
    Vector3f L_indir = castRay(Ray(x, wi), depth + 1) * inter.m->eval(wo, wi, N) * dotProduct(wi, N) / inter.m->pdf(wo, wi, N) / RussianRoulette;
    return L_dir + L_indir;
}