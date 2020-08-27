mergeInto(LibraryManager.library, {
    supplyPointer: function(arrPtr, length) {
        var animationIdArr = Module.HEAP32.subarray(arrPtr / 4, arrPtr / 4 + length);
        console.log(animationIdArr);

        Module['animationArrayCallback'](animationIdArr);
    },
});
