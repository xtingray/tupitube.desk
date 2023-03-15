#include <gphoto2/gphoto2-camera.h>

class Camera {
private:
    Camera *camera;
    GPContext *context;

public:
    Camera() {
        // Inicializar el contexto
        context = gp_context_new();

        // Inicializar la cámara
        camera = gp_camera_new();

        // Conectar la cámara
        gp_camera_init(camera, context);
    }

    ~Camera() {
        // Desconectar la cámara
        gp_camera_exit(camera, context);

        // Liberar memoria
        gp_camera_free(camera);
        gp_context_unref(context);
    }

    void captureImage(const char *filename) {
        CameraFile *file;
        CameraFilePath path;

        // Preparar la ruta para la imagen
        snprintf(path.folder, sizeof(path.folder), "/");
        snprintf(path.name, sizeof(path.name), "%s", filename);

        // Capturar la imagen
        gp_camera_capture(camera, GP_CAPTURE_IMAGE, &path, context);

        // Obtener el archivo de la imagen
        gp_file_new(&file);
        gp_camera_file_get(camera, path.folder, path.name, GP_FILE_TYPE_NORMAL, file, context);

        // Guardar el archivo de la imagen
        gp_file_save(file, filename);

        // Liberar memoria
        gp_file_free(file);
    }
};
