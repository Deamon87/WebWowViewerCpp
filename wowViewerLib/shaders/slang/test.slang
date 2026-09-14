#version 450

struct tmpstruct {
    MyStruct a;
    MyStruct b;
}

struct MyStruct {
    vec2 a;
    vec3 b;

    [mutating]
    void test() {
        this.a.xy = this.b.xy + this.a;
    }
}

struct ExampleArrays { 
   float2 before;
   float array[2];
   float2 after;
};
vec2 getElem(
    StructuredBuffer<MyStruct, ScalarDataLayout> arr,
    int idx 
) {
    return arr[idx].a; 
}


[[vk::binding(1,2)]] StructuredBuffer<MyStruct, ScalarDataLayout> input2;
[[vk::binding(1,3)]] RWStructuredBuffer<int, ScalarDataLayout> output;


[[vk::binding(1,4)]] RWStructuredBuffer<ExampleArrays, ScalarDataLayout> input3;


[[vk::binding(1,1)]] StructuredBuffer<ExampleArrays, Std140DataLayout> input;

layout(set = 0, binding = 0, std140) uniform UBO
{
    ExampleArrays a_test;
};

[shader("compute")]
[numthreads(1,1,1)]
void main() {
    static_assert(sizeof(input3[0]) == sizeof(input[0]), "oops");
}