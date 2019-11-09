unix {
INCLUDEPATH += $$MYPAINT_DIR/json-c $$MYPAINT_DIR/libmypaint $$MYPAINT_DIR/qtmypaint $$MYPAINT_DIR/raster/brushes $$MYPAINT_DIR/raster/main
LIBS += -L$$MYPAINT_DIR/json-c -L$$MYPAINT_DIR/libmypaint -L$$MYPAINT_DIR/qtmypaint -L$$MYPAINT_DIR/raster/brushes -L$$MYPAINT_DIR/raster/main -ljson-c -llibmypaint -lqtmypaint -lrasterbrushes -lrastermain
}

win32 {
INCLUDEPATH += $$MYPAINT_DIR/json-c $$MYPAINT_DIR/libmypaint $$MYPAINT_DIR/qtmypaint $$MYPAINT_DIR/raster/brushes $$MYPAINT_DIR/raster/main
LIBS += -L$$MYPAINT_DIR/json-c/release -L$$MYPAINT_DIR/libmypaint/release -L$$MYPAINT_DIR/qtmypaint/release -L$$MYPAINT_DIR/raster/main/release -ljson-c -llibmypaint -lqtmypaint -lrasterbrushes -lrastermain
}
