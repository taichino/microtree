#include <map>
#include "../microtree.h"

using namespace std;
using namespace microtree;

int main(int argc, char** argv) {
    tree t;
    tree::iterator head = t.begin();
    tree::iterator lang, os, lib, trash;
    tree::iterator cpp, python, as, win, japanese;

    // add some nodes
    lang  = t.insert(head, "Language");
    os    = t.insert(lang, "OperatingSystem");
    lib   = t.insert(os, "Library");
    trash = t.insert(lib, "Trash");
    cpp = t.add_child(lang, "C++");
    python = t.add_child(lang, "Python");
    as = t.add_child(lang, "ActionScript");
    japanese = t.add_child(lang, "Japanese");
    t.add_child(os, "mac");
    t.add_child(os, "linux");
    win = t.add_child(os, "windows");
    t.add_child(lib, "Django");
    t.add_child(lib, "OpenCV");

    // add some props to nodes
    cpp->props.insert(make_pair("author", string("Stroustrup")));
    cpp->props.insert(make_pair("birth", double(1980)));
    python->props.insert(make_pair("author", string("Guido")));
    python->props.insert(make_pair("birth", double(1992)));
    as->props.insert(make_pair("author", string("Adobe")));
    as->props.insert(make_pair("birth", double(1998)));

    // dump tree
    for (tree::iterator itr = t.begin(); itr != t.end(); itr++) {
	for (int d = 0; d < itr->depth(); d++) {
	    std::cout << "  ";
	}
	std::cout << *itr->key << "  " << itr->props << std::endl;
    }
    std::cout << std::endl;

    // erase some nodes
    t.erase(lib);
    t.erase(cpp);

    // move node
    t.move(trash, win, tree::TO_FIRSTCHILD);
    t.move(trash, japanese, tree::TO_FIRSTCHILD);
    t.move(trash, as, tree::TO_LASTCHILD);

    win = t.find("windows");
    t.erase(win);

    // dump again
    t.dump();
    
    return 0;
}
