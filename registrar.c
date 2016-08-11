#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <libfprint/fprint.h>

/* Descrubrir un dispositivo */
struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
	/* dispositivo */
	struct fp_dscv_dev *ddev = discovered_devs[0];
	/* driver */
	struct fp_driver *drv;
	if (!ddev)
		return NULL;
	/* obtener driver */
	drv = fp_dscv_dev_get_driver(ddev);
	printf("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
	return ddev;
}
/* Enrrolar huella, o guardar una nueva huella */
struct fp_print_data *enroll(struct fp_dev *dev) {
	struct fp_print_data *enrolled_print = NULL;
	int r;

	printf("You will need to successfully scan your finger %d times to "
		"complete the process.\n", fp_dev_get_nr_enroll_stages(dev));
/* Se hara hasta que el enrrolado sea completado, 5 veces exitosas */
	do {
		struct fp_img *img = NULL;

		sleep(1);
		printf("\nScan your finger now.\n");
		/* Para enrrolar una vez */
		r = fp_enroll_finger_img(dev, &enrolled_print, &img);
		if (img) {
			fp_img_save_to_file(img, "enrolled.pgm");
			printf("Wrote scanned image to enrolled.pgm\n");
			fp_img_free(img);
		}
		if (r < 0) {
			printf("Enroll failed with error %d\n", r);
			return NULL;
		}

		switch (r) {
		case FP_ENROLL_COMPLETE:
			printf("Enroll complete!\n");
			break;
		case FP_ENROLL_FAIL:
			printf("Enroll failed, something wen't wrong :(\n");
			return NULL;
		case FP_ENROLL_PASS:
			printf("Enroll stage passed. Yay!\n");
			break;
		case FP_ENROLL_RETRY:
			printf("Didn't quite catch that. Please try again.\n");
			break;
		case FP_ENROLL_RETRY_TOO_SHORT:
			printf("Your swipe was too short, please try again.\n");
			break;
		case FP_ENROLL_RETRY_CENTER_FINGER:
			printf("Didn't catch that, please center your finger on the "
				"sensor and try again.\n");
			break;
		case FP_ENROLL_RETRY_REMOVE_FINGER:
			printf("Scan failed, please remove your finger and then try "
				"again.\n");
			break;
		}
	} while (r != FP_ENROLL_COMPLETE); // Saldra cuando se completen las 5 enrolamientos exitosos

	if (!enrolled_print) {
		fprintf(stderr, "Enroll complete but no print?\n");
		return NULL;
	}

	printf("Enrollment completed!\n\n");
	return enrolled_print;
}

/************************************
DEF
*/
/*save*/
int save(struct fp_print_data *data, int user, struct fp_dev *dev){
	sqlite3 * db;
	char * sql;
	int rc;
	int len,error;
	unsigned char *buf;
	sqlite3_stmt *res;

	// se transforman las dos datas, de dos huellas introducidas, a dos buffers
	len = fp_print_data_get_data(data, &buf);
//Se abre la BD
	rc = sqlite3_open("usuariosH.db", &db);
		if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }
//SE inserta en la tabla, id y huella
	sql = "INSERT INTO Huellas (user_id,huella,size_data) VALUES (?,?,?)";
//Se prepara el statement
	rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
//Si el statement es correcto, introducimos en id y huella sus correspondiente valores
	if (rc == SQLITE_OK) {
		sqlite3_bind_int(res, 1, user);
		sqlite3_bind_blob(res, 2, buf,len,0);
    sqlite3_bind_int(res, 3,len);
	} else {
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
	}
	//Hacemos que funciona el statement
	int step = sqlite3_step(res);
	//Se finaliza el statement
	sqlite3_finalize(res);
	//Se cierra BD
	sqlite3_close(db);
}

void main(int argc, char *argv[])
{
	int r = 1, error = 0;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;
	struct fp_print_data *data;

	printf("El programa registrara una huella para el usuario %d \n presiona enter, de otro modo presiona "
		"Ctrl+C \n", atoi(argv[1]));
	getchar();

	r = fp_init();
	if (r < 0) {
		fprintf(stderr, "Failed to initialize libfprint\n");
		exit(1);
	}
	fp_set_debug(3);

	discovered_devs = fp_discover_devs();
	if (!discovered_devs) {
		fprintf(stderr, "Could not discover devices\n");
		goto out;
	}

	ddev = discover_device(discovered_devs);
	if (!ddev) {
		fprintf(stderr, "No devices detected.\n");
		goto out;
	}

	dev = fp_dev_open(ddev);
	fp_dscv_devs_free(discovered_devs);
	if (!dev) {
		fprintf(stderr, "Could not open device.\n");
		goto out;
	}

	printf("Opened device. It's now time to enroll your finger.\n\n");

	data = enroll(dev);
	if (!data)
		goto out_close;

	error = save(data,atoi(argv[1]),dev);
	if (error)
		goto out_close;

	printf("Data guardada en BD.\n\n");

	fp_print_data_free(data);
out_close:
	fp_dev_close(dev);
out:
	fp_exit();

}
