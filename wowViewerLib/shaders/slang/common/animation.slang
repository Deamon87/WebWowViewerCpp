import glsl;

struct M2PartTrackElement {
    int time;
    int valueIndx; // Index into one of the arrays (vec3, vec2, float)
};

struct M2AnimTrack {
    int elemCount;
    int m2PartTrackElementIndex;
};

struct M2Track {
    uint interpolation_type;
    int global_sequence;
    int m2AnimTrackIdx;
};

float calcAlpha(in float ftime,
                in M2PartTrackElement val1,
                in M2PartTrackElement val2) {

    return (ftime - val1.time) / (val2.time - val1.time);
}


//fTime is in ms
void getAnimationKeys(in float ftime,
                 in int elemSize,
                 in const StructuredBuffer<M2PartTrackElement, ScalarDataLayout> elems,
                 out float alpha,
                 out int valueIdx1,
                 out int valueIdx2) {
    if (elemSize == 1) {
        int elemIdx = elems[0].valueIndx;
        valueIdx1 = elemIdx;
        valueIdx2 = elemIdx;
        alpha = 0;

        return;
    } else if (elemSize > 1) {
        for (int i = 0; i < elemSize - 1; i++) {
            const M2PartTrackElement rec1 = elems[i];
            const M2PartTrackElement rec2 = elems[i+1];
            if (ftime >= rec1.time && rec2.time <= ftime) {
                valueIdx1 = rec1.valueIndx;
                valueIdx2 = rec2.valueIndx;
                alpha = calcAlpha(ftime, rec1, rec2);

                return;
            }
        }

        const M2PartTrackElement lastRec = elems[elemSize - 1];
        valueIdx1 = lastRec.valueIndx;
        valueIdx2 = lastRec.valueIndx;
        alpha = 0;
    }

    valueIdx1 = -1;
}

vec3 animatePartTrack(in float age,
                      in int elemSize,
                      in StructuredBuffer<M2PartTrackElement, ScalarDataLayout> elems,
                      in StructuredBuffer<vec3, ScalarDataLayout> values,
                      in vec3 defaultValue
) {
    int valueIdx1, valueIdx2;
    float alpha;
    getAnimationKeys(age, elemSize, elems, alpha, valueIdx1, valueIdx2);
    if (valueIdx1 > 0) {
        return mix(values[valueIdx1], values[valueIdx2], alpha);
    }

    return defaultValue;
}

vec2 animatePartTrack(in float age,
                      in int elemSize,
                      in StructuredBuffer<M2PartTrackElement, ScalarDataLayout> elems,
                      in StructuredBuffer<vec2, ScalarDataLayout> values,
                      in vec2 defaultValue
) {
    int valueIdx1, valueIdx2;
    float alpha;
    getAnimationKeys(age, elemSize, elems, alpha, valueIdx1, valueIdx2);
    if (valueIdx1 > 0) {
        return mix(values[valueIdx1], values[valueIdx2], alpha);
    }

    return defaultValue;
}

float animatePartTrack(in float age,
                      in int elemSize,
                      in StructuredBuffer<M2PartTrackElement, ScalarDataLayout> elems,
                      in StructuredBuffer<float, ScalarDataLayout> values,
                      in float defaultValue
) {
    int valueIdx1, valueIdx2;
    float alpha;
    getAnimationKeys(age, elemSize, elems, alpha, valueIdx1, valueIdx2);
    if (valueIdx1 > 0) {
        return mix(values[valueIdx1], values[valueIdx2], alpha);
    }

    return defaultValue;
}