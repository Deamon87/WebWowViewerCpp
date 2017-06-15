fs = require('fs');
var glsl = require('glsl-man');


function uniqBy(a, key) {
    var seen = new Set();
    return a.filter(item => {
        var k = key(item);
        return seen.has(k) ? false : seen.add(k);
    });
}

fs.readFile(process.argv[2], 'utf8', function (err,data) {
    if (err) {
        return console.log(err);
    } else {
        var ast = glsl.parse(data);
        var selector = null;
        if (process.argv[3] == "attribute") {
            selector = 'declarator[typeAttribute] > type[qualifier=attribute]'
        } else if (process.argv[3] == "uniform") {
            selector = "declarator[typeAttribute] > type[qualifier=uniform]"
        }

        var names = glsl.query.all(
            ast,
            glsl.query.selector(selector));

        var index = 0;
        uniqBy(names, (a) => {return a.parent.declarators[0].name.name}).map(a => {
            console.log(a.parent.declarators[0].name.name, index, a.name)
            if (a.name == "mat4") {
                index = index + 4;
            } else if (a.name == "mat3"){
                index = index + 3;
            } else {
                index = index + 1
            }
        });
    }
});
