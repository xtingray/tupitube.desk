unix {
INCLUDEPATH += $$MYPAINT_DIR $$MYPAINT_DIR/json-c $$MYPAINT_DIR/libmypaint $$MYPAINT_DIR/qtmypaint $$MYPAINT_DIR/raster
LIBS += -L$$MYPAINT_DIR/json-c -L$$MYPAINT_DIR/libmypaint -L$$MYPAINT_DIR/qtmypaint -L$$MYPAINT_DIR/raster -ljson-c -llibmypaint -lqtmypaint -lrastercanvas
}

win32 {
INCLUDEPATH += $$MYPAINT_DIR $$MYPAINT_DIR/json-c $$MYPAINT_DIR/libmypaint $$MYPAINT_DIR/qtmypaint $$MYPAINT_DIR/raster
LIBS += -L$$MYPAINT_DIR/json-c/release -L$$MYPAINT_DIR/libmypaint/release -L$$MYPAINT_DIR/qtmypaint/release -L$$MYPAINT_DIR/raster/release -ljson-c -llibmypaint -lqtmypaint -lrastercanvas
}
