// Declaración de la estructura de datos // de una huella captura por el dispositivo

//struct fp_print_data *fdata;

// Coloca en un buffer la información capturada // por el dispositivo y devuelve la longitud

//len = fp_print_data_get_data(data, &buf);

// Convierte los datos almacenados en un // buffer en la estructura de datos fdata

//fdata = fp_print_data_from_data(buf,len);

// Esta es la más importante // toma una huella y realiza la comparación // entre una colección de huellas, almacenada // en fdata que, en este caso, debe ser // un array de punteros

//fp_identify_finger(dev, fdata, &posicion);


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libfprint/fprint.h>

struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
	struct fp_dscv_dev *ddev = discovered_devs[0];
	struct fp_driver *drv;
	if (!ddev)
		return NULL;

	drv = fp_dscv_dev_get_driver(ddev);
	printf("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
	return ddev;
}

struct fp_print_data *enroll(struct fp_dev *dev) {
	struct fp_print_data *enrolled_print = NULL;
	int r;

	printf("You will need to successfully scan your finger %d times to "
		"complete the process.\n", fp_dev_get_nr_enroll_stages(dev));

	do {
		struct fp_img *img = NULL;

		sleep(1);
		printf("\nScan your finger now.\n");

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
	} while (r != FP_ENROLL_COMPLETE);

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
int verify(struct fp_dev *dev, struct fp_print_data **data)
{
	int r;
	size_t offset = 0;

	do {
		struct fp_img *img = NULL;

		sleep(1);
		printf("\nScan your finger now.\n");
		r = fp_identify_finger(dev, data, &offset);

		printf("OFFSET; %d\n", offset);
		//r = fp_verify_finger(dev, data);
		if (img) {
			fp_img_save_to_file(img, "verify.pgm");
			printf("Wrote scanned image to verify.pgm\n");
			fp_img_free(img);
		}
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
			return 0;
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

typedef struct fp_user {
	  unsigned char *buf;
		int user;
		int len;
} Users;

Users users[5];
//struct fp_print_data *fdata;


void get(struct fp_dev *dev){
	unsigned char *buffer;
	int n = 5
		struct fp_print_data ** fdata = (struct fp_print_data **) calloc(n,sizeof(struct fp_print_data *));

			//Se abre BD
			rc = sqlite3_open("test.db", &db);
			//Se hace un sql de select
			sql = "SELECT DISTINCT * FROM Huellas";
			//Preparacion de statement
			rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

			 step = sqlite3_step(res);
			 int i = 0;
			while (i != 3) {

					printf("i:%d    %s: ",i, sqlite3_column_text(res, 0));
					printf("%s\n", sqlite3_column_text(res, 1));

					fdata[i] = fp_print_data_from_data((unsigned char *)sqlite3_column_blob(res, 1),len);
					i++;
			}
			//buf = sqlite3_column_blob(res, 1);

			//Anadimos en el arreglo de fdata (fp_print_data) las huellas leidas de la BD
			fdata[0] = fp_print_data_from_data(buf,len); //FUNCIONA
			fdata[1] = fp_print_data_from_data(buf1,len1);
			fdata[2] = fp_print_data_from_data(buf2,len2);
			//error = 0;
			//Se verifica (IDENTIFICA), mandandole el dispositivo y el arreglo de fdata
			do{
			  verify(dev,fdata);
			}while(1);

			sqlite3_finalize(res);
			sqlite3_close(db);
}

void getAll(){

}


int main(void)
{
	int r = 1, error = 0;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;
	struct fp_print_data *data, *data1;

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

	get(dev);

	//fp_print_data_free(data);
	out_close:
		fp_dev_close(dev);
	out:
		fp_exit();
		return r;

}
