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
		/* Para enrolar una vez */
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
/* Identificacion de una huella entre varias huellas */
int identify(struct fp_dev *dev, struct fp_print_data **data)
{
	int r;
	size_t offset = 0;
//Se hara siempre hasta que se encuentre que MATCH(hubo compatible) o NO MATCH (no compatible)
	do {
		struct fp_img *img = NULL;

		sleep(1);
		printf("\nScan your finger now.\n");
		/* IDENTIFY, de uno entre varios */
		r = fp_identify_finger(dev, data, &offset);

		printf("OFFSET; %d\n", offset);

		if (r < 0) {
			printf("verification failed with error %d :(\n", r);
			return r;
		}
		switch (r) {
		case FP_VERIFY_NO_MATCH:
			printf("NO MATCH!\n");
			return 0;
		case FP_VERIFY_MATCH:
			printf("MATCH!\n");
			return 1;
		case FP_VERIFY_RETRY:
			printf("Scan didn't quite work. Please try again.\n");
			break;
		case FP_VERIFY_RETRY_TOO_SHORT:
			printf("Swipe was too short, please try again.\n");
			break;
		case FP_VERIFY_RETRY_CENTER_FINGER:
			printf("Please center your finger on the sensor and try again.\n");
			break;
		case FP_VERIFY_RETRY_REMOVE_FINGER:
			printf("Please remove finger from the sensor and try again.\n");
			break;
		}
	} while (1);
}

struct fp_print_data ** getAll(struct fp_dev *dev){
  sqlite3 * db;
	char * sql;
	int rc;
	int n = 5,step;
  sqlite3_stmt *res;
	struct fp_print_data ** fdata = (struct fp_print_data **) calloc(n,sizeof(struct fp_print_data *));

		//Se abre BD
		rc = sqlite3_open("usuariosH.db", &db);
		//Se hace un sql de select
		sql = "SELECT DISTINCT * FROM huellas";
		//Preparacion de statement
		rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

		int i = 0;
		//Guardara en el arreglo fdata las huellas, y mostrara en pantalla las columnas de la BD
		while ((step = sqlite3_step(res)) == SQLITE_ROW) {

				printf("i:%d    id: %s ",i, sqlite3_column_text(res, 0));
				printf("user_id: %s   ", sqlite3_column_text(res, 1));
        printf("DATA: %s    size_data: %s\n", sqlite3_column_text(res, 2),sqlite3_column_text(res, 3));

				fdata[i] = fp_print_data_from_data((unsigned char *)sqlite3_column_blob(res, 2),sqlite3_column_int(res,3));
				i++;
		}
  /*  do{
  	  identify(dev,fdata);
  	}while(1);*/

		sqlite3_finalize(res);
		sqlite3_close(db);
    return fdata;
}

void main(int argc, char *argv[])
{
	int r = 1, error = 0;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;
  int n = 5;
  struct fp_print_data ** fdata = (struct fp_print_data **) calloc(n,sizeof(struct fp_print_data *));

	printf("Este programa identificara si la huella existe y de que usuario \n\n");

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

  fdata = getAll(dev);
  do{
	  identify(dev,fdata);
	}while(1);


	//fp_print_data_free(data);
out_close:
	fp_dev_close(dev);
out:
	fp_exit();

}
