var fs = require("fs");
var json = fs.readFileSync(process.argv[2], "utf8");
var nnum = 0;

var node = JSON.parse(json);

function nodeToDot(node) {
    if (!("id" in node)) node.id = nnum++;
    console.log("node_" + node.id + " [label=\"" + node.type + "\"];");

    if (node.tok) {
        console.log("node_" + node.id + " -> node_tok_" + node.id + ";");
        console.log("node_tok_" + node.id + " [label=\"" + node.tok.tok + "\"];");
    }

    for (var i = 0; i < node.children.length; i++) {
        var c = node.children[i];
        if (!("id" in c)) c.id = nnum++;
        console.log("node_" + node.id + " -> node_" + c.id + ";");
        nodeToDot(c);
    }
}

console.log("digraph {");
nodeToDot(node);
console.log("}");
