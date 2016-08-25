// 2015-09-29: maru+shimo version
#include <stdio.h>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>
#include <direct.h>
#include <conio.h> // for windows only kbhit and getch


/*�ŏ��̃L�[�{�[�h�����\��*/
void D0(void)		//�L�[����ꗗ�̃e�L�X�g�\��
{
	fprintf(stderr,
		"--------------------------------------------------------------------------\n"
		" 1 :�e���v���[�g�p�摜�̕ۑ�.\n"
		" 2 :�e���v���[�g�p�摜�̕ҏW�E�g���~���O.\n"
		" 3 :�摜��250���ۑ�(Spece Key�̓��͂ŏI��).\n"
		" 4 :�e���v���[�g�}�b�`���O�̎��s,���ʃf�[�^�o��.\n"
		" 5 :���ʃf�[�^�̉��.\n"
		" 6 :�f�[�^�ۑ�.\n"
		" 0 :���̃A�v���P�[�V�������I������\n"
		" 7 :�}�b�`���O�̗ގ��x��ύX����\n"
		" 8 :�f�[�^�R�s�[�̂��߃t�H���_���J��\n"
		" 9 :�����摜��\������\n"
		" �����t�H���_�����邱�Ƃ��m�F���āA�B�e�E��͂��s���Ă�������.\n"
		"-------------------------------------------------------------------------\n"
		);
}
void D00(void)		//�L�[����ꗗ�̃e�L�X�g�\��
{
	fprintf(stderr,
		"----�o�[�W����("__DATE__"-"__TIME__")-----------------------------------\n"
		" h or ? : ���j���[�̕\��\n"
		" c :�J�������͂̕ύX\n"
		" t :�������ɉ摜��ۑ�(Spece Key�̓��͂ŏI��)\n"
		" R :�e���v���[�g�̗��K(Space �I���EENTER �V���[�h)\n"
		);
	fprintf(stderr, " d :�����t�H���_�̍쐬.\n");
	fprintf(stderr, " b :�e���v���[�g�p�摜�̕ۑ�.\n");
	fprintf(stderr, " s :�摜��250���ۑ�\n");

	fprintf(stderr, " m :Match Template�@�̎��s,���l�o��.\n");
	fprintf(stderr, " q or Esc : ���̃A�v���P�[�V�������I������\n");
	fprintf(stderr, "-------------------------------------------------------------------------\n\n");
}

//���ԏo�͗p
#include <time.h>
#include <windows.h>

/*OpenCV 2.4.9�̃��C�u����*/
#ifdef _DEBUG
#pragma comment (lib,    "opencv_core249d.lib")
#pragma comment (lib, "opencv_imgproc249d.lib")
#pragma comment (lib, "opencv_highgui249d.lib")
#else
#pragma comment (lib,    "opencv_core249.lib")
#pragma comment (lib, "opencv_imgproc249.lib")
#pragma comment (lib, "opencv_highgui249.lib")
#endif

#pragma comment(lib, "shlwapi.lib")

#pragma comment(lib,"user32.lib")

#define DEFAULT_CONFIG	0.70 //�ގ��x�̏����ݒ�
#define FILECOUNT		250
#define FILECOUNT_MAX   9999
//100����3�b���x	(30fps)	//��������������߂���ƃv���O�����Ŏg�p����z��̃T�C�Y���ς��
//�o�b�t�@�T�C�Y�����肸���s�ł��Ȃ����Ƃ�����


/* �O���[�o���ϐ� */
//	IplImage *img = 0;
const char* FolderName = "�����t�H���_";//�t�H���_��
errno_t error;
using namespace std;

void D0(void);
void D(int K);

//�����擾�֐� windows �ˑ�
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib") 
DWORD timeGettime_Start;
double Ftime[FILECOUNT_MAX];
int Ftime_max;
void startClock(){
	Ftime_max = 0;
	timeGettime_Start = timeGetTime();
}
double measureClock(){
	DWORD time = timeGetTime();
	return((double)(time - timeGettime_Start) / 1000);
}


void saveClock(int i){
	Ftime[i] = measureClock();
	Ftime_max = i + 1;
}
char charTimes[20];
char* sprintClock(int i){
	sprintf_s(charTimes, "%.3lf", Ftime[i]);
	return(charTimes);
}
char* sprintClockwPer(int i, int p){
	sprintf_s(charTimes, "%.3lfs (%d%%)", Ftime[i], p);
	return(charTimes);
}

void fprintClock(void){
	char strS[_MAX_PATH] = "";
	FILE* file_s;
	sprintf_s(strS, "%s\\���l�f�[�^\\���ԃf�[�^.csv", FolderName);
	//���Ԑ��l�f�[�^�̍쐬
	if ((error = fopen_s(&file_s, strS, "w")) != 0) {
		printf("error\n");
	}
	for (int j = 0; j < Ftime_max; j++){
		fprintf(file_s, "%lf\n", Ftime[j]);		//���Ԃ𐔒l�f�[�^�ɏo��
	}
	fclose(file_s);
	fprintf(stderr, "���ԃf�[�^���o�͂��܂���.\n");

}
int fscanClock(void){
	FILE *file_csv;
	char strT[_MAX_PATH] = "";
	sprintf_s(strT, "%s\\���l�f�[�^\\���ԃf�[�^.csv", FolderName);
	if (error = fopen_s(&file_csv, strT, "r") != 0){
		printf("%s\n", strT);
		return(-1);
	}
	int maxline = 0;
	int ierr;
	while (maxline<FILECOUNT_MAX && (ierr = fscanf_s(file_csv, "%lf", &Ftime[maxline])) >0){
		maxline++;
		//		printf("%d: scan%d\n", maxline,ierr);
		if (ierr == 0) break;
		if (ierr == EOF){
			maxline--;
			break;
		}
	}
	fclose(file_csv);
	Ftime_max = maxline + 1;
	return(maxline);
}

int main(int argc, char **argv)
{

	int i, test, max = 0, M;
	double min_val, max_val;
	double Cmax, Cmin;
	double config_val;//Config.txt ����ǂݍ��ޗގ��x
	CvPoint min_loc, max_loc;
	CvPoint Pmin, Pmax;
	CvSize dst_size;
	IplImage *src_img, *dst_img;
	IplImage *img_ccoeff;


	//-------------------------------------------------------------------------------
	char strB[_MAX_PATH] = "";
	char strS[_MAX_PATH] = "";
	char strT[_MAX_PATH] = "";
	char strR[_MAX_PATH] = "";

	//-------------------------------------------------------------------------------

	int Savecount = -1;//���f�����p�ϐ�

	//���l�f�[�^�t�@�C�����o��
	FILE *file, *file_config;

	//�t�H���g - time stamp
	CvFont font, font_w;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 1.0, 0, 1, CV_AA);
	cvInitFont(&font_w, CV_FONT_HERSHEY_SIMPLEX, 0.5, 1.0, 0, 3, CV_AA);

	int defaultCAM = 1;// ThinkPad �J�����p�̐ݒ�
	CvCapture * videoCapture1 = cvCaptureFromCAM(defaultCAM);
	//�P��݂̂̏ꍇcvCaptureFromCAM�̈����͂Ȃ�ł�����
	//������̏ꍇ��PC�N�����̐ڑ����������ɂȂ�


	//------------------------------
	D0();
	if (_mkdir("�����t�H���_") == 0){													//.exe�Ɠ����K�w�Ƀf�B���N�g�����쐬
		printf("�t�H���_�쐬\n");
	}
	else{
		printf("�t�H���_�쐬�Ɏ��s���܂����B���Ƀt�H���_�����݂���\��������܂��B\n");
	}
	_mkdir("�����t�H���_\\���l�f�[�^");
	_mkdir("�����t�H���_\\�B�e�摜");
	_mkdir("�����t�H���_\\�����摜");


	if (error = fopen_s(&file_config, "./�����t�H���_/Config.txt", "r") != 0){
		fopen_s(&file_config, "./�����t�H���_/Config.txt", "w");
		fprintf(file_config, "%f", DEFAULT_CONFIG);
		config_val = DEFAULT_CONFIG;
		fclose(file_config);
	}
	else{
		fscanf_s(file_config, "%lf", &config_val);
		fclose(file_config);
		fprintf(stderr, "\n�ގ��x%.1f%%�ɐݒ肵�܂���.\n", config_val*100.0);
	}
	fprintf(stderr, "\n�����t�H���_�쐬���J���܂�.\n");
	

	//�J�������J��-----------------
	cv::VideoCapture cap;
	cap.open(0);

	if (!cap.isOpened())	{
		printf("\n\n\n\n�G���[�F�J������F���ł��܂���ł���.\n�I�����܂�.\n\n");
		getchar();
		cv::waitKey(10000);
		return -1;
	}

	//���ݑ��֌W�����z�𐶐����邽�߂̕ϐ�
	img_ccoeff = cvCreateImage(cvSize(640, 480), IPL_DEPTH_32F, 1);
	cv::Mat frame;
	while (1){
		cap >> frame;

		int key = cv::waitKey(1);
		if (key == -1 && _kbhit()){
			key = _getch();
			printf("\n>");
			putchar(key);
			printf("\n");
		}


		//�E�B���h�E����------------------------------------------------------------
		cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE);
		IplImage *image1 = cvQueryFrame(videoCapture1);
		cvShowImage("Camera", image1);
		//---------------------------------------------------------------------------------------------------------------
		if (key == 'c'){
			defaultCAM--;
			if (defaultCAM < 0){ defaultCAM = 3; }
			printf("\n\n�J�����ԍ���%02d�֕ύX���܂���.\n��ʂɓ����������OK�ł�\n", defaultCAM);
			cvReleaseCapture(&videoCapture1);
			videoCapture1 = cvCaptureFromCAM(defaultCAM);
		}
		//���ݑ��֌W�����z
		else if (key == 'C'){
			cvConvertScale(img_ccoeff, img_ccoeff, 1.0 / Cmax, 0.0);
			cvShowImage("���ݑ��֌W�����z", img_ccoeff);
		}
		//---------------------------------------------------------------------------------------------------------------
		else if (key == 'f' || key == '2'){
			fprintf(stderr, "�e���v���[�g.bmp���J���܂��̂ł��҂����������B\n");
			system("mspaint \".\\�����t�H���_\\�e���v���[�g.bmp\"");												//�y�C���g�őΏۂ��J��
			D(key);
		}
		else if (key == '5'){
			fprintf(stderr, "���ʃf�[�^.csv���J���܂��̂ł��҂����������B\n");
			system("explorer \".\\�����t�H���_\\���l�f�[�^\\���ʃf�[�^.csv\"");												//�G�N�X�v���[���[�őΏۂ��J��
			D(key);
		}
		else if (key == '6'){
			system("xcopy /S /C /I /Y \".\\�����t�H���_\" %date:~-5,2%%date:~-2%%time:~0,2%%time:~3,2%%time:~6,2%");
			D(key);
		}
		else if (key == '7'){
			fprintf(stderr, "Config.txt���J���܂��B�ގ��x��ݒ肵�Ă��������B\n");
			system("explorer \".\\�����t�H���_\\Config.txt\"");												//�G�N�X�v���[���[�őΏۂ��J��
			D(key);
		}
		else if (key == '8'){
			system("explorer \".\\�����t�H���_\"");												//�G�N�X�v���[���[�őΏۂ��J��
			D(key);
		}
		else if (key == 'd' || key == 'D'){ //C:\\Users\\yama\\Documents\\�����t�H���_-webCam
			if (_mkdir("�����t�H���_") == 0){													//.exe�Ɠ����K�w�Ƀf�B���N�g�����쐬
				printf("�t�H���_�쐬\n");
			}
			else{
				printf("�t�H���_�쐬�Ɏ��s���܂����B���Ƀt�H���_�����݂���\��������܂��B\n");
			}
			system("explorer \"�����t�H���_");												//�G�N�X�v���[���[�őΏۂ��J��
			_mkdir("�����t�H���_\\���l�f�[�^");
			_mkdir("�����t�H���_\\�B�e�摜");
			_mkdir("�����t�H���_\\�����摜");
			fprintf(stderr, "�����p�t�H���_�쐬���J���܂�.\n");
			D(key);

		}
		//�摜��1���̕ۑ����s��------------------------------------------------------------------------------
		else if (key == 'b' || key == 'B' || key == '1'){
			IplImage *output = cvQueryFrame(videoCapture1);

			sprintf_s(strB, "%s\\�e���v���[�g.bmp", FolderName);
			cvSaveImage(strB, output);

			fprintf(stderr, "�e���v���[�g�p�摜�̕ۑ��ɐ������܂���\n");
			D(key);
		}
		//�ݒ薇�����̉摜���������g�p���ۑ�����------------------------------------------------------------------------
		else if (key == 's' || key == '3'){
			fprintf(stderr, "�摜���������Ɋi�[���܂�.(Space key �ŏI��).\n");
			cvDestroyAllWindows();
			IplImage *output;
			IplImage** vout;
			vout = (IplImage **)malloc(sizeof(IplImage *)*FILECOUNT);
			int i;
			startClock();
			for (i = 0; i < FILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space key�����͂���܂����B�L�^���I�����܂��B");
					break;
				}

				cap >> frame;
				output = cvQueryFrame(videoCapture1);
				vout[i] = cvCloneImage(output);
				saveClock(i);
				printf("%04d (Space key �ŏI��)\n", i);
			}
			int imax = i;
			fprintf(stderr, "�摜���f�B�X�N�ɕۑ����܂�.\n");

			for (int i = 0; i < imax; i++){
				sprintf_s(strS, "%s\\�B�e�摜\\outputpic_%04d.bmp", FolderName, i);
				if (vout[i] == NULL) continue;
				cvShowImage("Camera", vout[i]);
				cvWaitKey(2);
				cvSaveImage(strS, vout[i]);
				printf("outputpic_%04d.bmp �ۑ�\n", i);
				cvReleaseImage(&(vout[i]));
			}
			free(vout);

			fprintf(stderr, "�B�e�摜��ۑ����܂���.\n");
			fprintClock();
			//--------------------------------------------------------------------------
			cvShowImage("Camera", image1);
			D(key);
		}
		//�ݒ薇�����̉摜���������g�p�������ɕۑ�����------------------------------------------------------------------------
		else if (key == 't'){
			fprintf(stderr, "�摜�������i�[�J�n.(Space key �ŏI��).\n");
			cvDestroyAllWindows();
			IplImage *output;
			IplImage** vout;
			int mFILECOUNT = FILECOUNT_MAX;
			vout = (IplImage **)malloc(sizeof(IplImage *)*mFILECOUNT);
			int i;
			startClock();
			for (i = 0; i < mFILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space key�����͂���܂����B�L�^���I�����܂��B");
					break;
				}
				cap >> frame;
				output = cvQueryFrame(videoCapture1);
				vout[i] = cvCloneImage(output);
				if (vout[i] == NULL){
					printf("�������i�[�Ɏ��s���܂����B�L�^���I�����܂��B");
					break;
				}
				saveClock(i);
				printf("%04d �������i�[\n", i);
			}
			int imax = i;
			fprintf(stderr, "�摜�ۑ��J�n.\n");

			for (int i = 0; i < imax; i++){
				sprintf_s(strS, "%s\\�B�e�摜\\outputpic_%04d.bmp", FolderName, i);
				if (vout[i] == NULL) continue;
				cvSaveImage(strS, vout[i]);
				printf("outputpic_%04d.bmp �ۑ�\n", i);
				cvReleaseImage(&(vout[i]));
			}
			free(vout);

			fprintf(stderr, "�B�e�摜��ۑ����܂���.\n");
			fprintClock();
			//--------------------------------------------------------------------------
			cvShowImage("Camera", image1);
			D(key);
		}
		//�ݒ薇�����̉摜��ۑ�����------------------------------------------------------------------------
		else if (key == 'S'){
			fprintf(stderr, "�摜�ۑ��J�n.\n");
			cvDestroyAllWindows();
			startClock();
			for (int i = 0; i < FILECOUNT; i++){
				cap >> frame;

				IplImage *output = cvQueryFrame(videoCapture1);
				sprintf_s(strS, "%s\\�B�e�摜\\outputpic_%04d.bmp", FolderName, i);
				saveClock(i);
				cvSaveImage(strS, output);

				printf("outputpic_%04d.bmp �ۑ�\n", i);
			}

			fprintf(stderr, "�B�e�摜��ۑ����܂���.\n");
			fprintClock();

			cvShowImage("Camera", image1);
			D(key);
		}
		//�������ɕۑ�����--------------------HDD�̈悪��������L�^�ł���͂�
		else if (key == 'T'){

			//�ۑ���Camera�E�B���h�E���������ɂȂ�̂ŃE�B���h�E�������Ă���
			cvDestroyAllWindows();

			Savecount = 0;
			fprintf(stderr, "�������ɕۑ����� Space key�ŏI��");
			startClock();
			for (int i = 0; i < FILECOUNT_MAX; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space key�����͂���܂����B�L�^���I�����܂��B");
					break;
				}

				cap >> frame;
				//IplImage output = frame;
				IplImage *output = cvQueryFrame(videoCapture1);

				sprintf_s(strR, "%s\\�B�e�摜\\outputpic_%04d.bmp", FolderName, i);
				saveClock(i);
				cvSaveImage(strR, output);
				printf("outputpic_%04d.bmp :", i);
				i++;
			}

			//Camera�E�B���h�E�ĕ\��
			cvShowImage("Camera", image1);
			fprintClock();
			fprintf(stderr, "�摜�̕ۑ��ɐ������܂���!\n");
			D(key);
		}
		else if (key == 'm' || key == '4'){
			//�}�b�`�e���v���[�g
			int num_bmp = 0;
			int num_effected = 0;
			IplImage *tmp_img;
			double val4files[FILECOUNT_MAX];//�ގ��x
			char XYpoint[FILECOUNT_MAX][20];//���W�擾�̕�����20����
			for (int i1 = 0; i1 < FILECOUNT_MAX; i1++){
				val4files[i1] = -0.01;
			}
			printf("\n\n��͏������s���܂�.\n");
			// precheck for ���ʃf�[�^
			sprintf_s(strR, "%s\\���l�f�[�^\\���ʃf�[�^.csv", FolderName);
			if (error = fopen_s(&file, strR, "w") != 0){
				printf("%s\n", strR);
				fprintf(stderr, "���ʃf�[�^�ɏ������߂܂���.�����𒆒f���܂�.\n");
				fprintf(stderr, "�iExcel�ȂǂŊJ���Ă���Ə������߂܂���̂ŕ��Ă�������.�j\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}
			else{ fclose(file); }

			//for csv
			int myFILECOUNT = fscanClock();
			if (myFILECOUNT == -1){
				fprintf(stderr, "�B�e�f�[�^������܂���.\n");
				fprintf(stderr, "�B�e���s���Ă�������.\n\n");
				cvShowImage("Camera", image1);
				D('b');
				continue;
			}
			//--------------------------------------------------------------------------------------
			printf("%d�̃e���v���[�g�}�b�`���O�������s���܂��B���X���҂����������B\n", myFILECOUNT);

			//�ǂݍ���
			sprintf_s(strR, "%s\\�e���v���[�g.bmp", FolderName);						//template picture as bmp format
			tmp_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

			//�ǂݍ��ݎ��s
			if (tmp_img == NULL) {
				fprintf(stderr, "�e���v���[�g�摜�̓Ǎ��݂Ɏ��s���܂���.\n");
				fprintf(stderr, "�e���v���[�g�摜���쐬���Ă�������.\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}
			//read config.txt
			if (error = fopen_s(&file_config, "./�����t�H���_/Config.txt", "r") != 0){
				fprintf(stderr, "Config.txt �̓Ǎ��݂Ɏ��s���܂���.\n");
				fprintf(stderr, "�ċN�����Ă�������.\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;

			}
			else{
				fscanf_s(file_config, "%lf", &config_val);
				fclose(file_config);
			}
			bool use_src_img = FALSE;
			for (i = 0; i < myFILECOUNT; i++){
				sprintf_s(strR, "%s\\�B�e�摜\\outputpic_%04d.bmp", FolderName, i);
				if (use_src_img){
					cvReleaseImage(&src_img);
				}
				src_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
				use_src_img = TRUE;

				//�ǂݍ��ݎ��s
				if (src_img == NULL) {
					printf("�B�e�摜�̓Ǎ��݂Ɏ��s���܂���.\n");
				}
				else{
					num_bmp++;
					printf("outputpic_%04d.bmp:", i);
					// (1)�T���摜�S�̂ɑ΂��āC�e���v���[�g�̃}�b�`���O�l�i�w�肵����@�Ɉˑ��j���v�Z
					dst_size = cvSize(src_img->width - tmp_img->width + 1, src_img->height - tmp_img->height + 1);
					dst_img = cvCreateImage(dst_size, IPL_DEPTH_32F, 1);
					cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);				//method���ς����؂��K�v

					cvMinMaxLoc(dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);

					/*150�Ԗڂ̗ގ��x�m�F�p*/
					if (i == 150){
						img_ccoeff = dst_img;
						cvMinMaxLoc(img_ccoeff, &Cmin, &Cmax, &Pmin, &Pmax, NULL);

					}
					cvReleaseImage(&dst_img);
					val4files[i] = max_val;

					if (val4files[i] < config_val){//�e���v���[�g�ƒT���Ώۂ̗ގ��x��config_val ����
						//X,Y���W�̏o�� - for csv
						sprintf_s(XYpoint[i], ",%02d,%02d", 0, 0);
						printf("�ގ��x%.1f(%.1f%%����)\n", max_val*100.0, config_val*100.0);

						cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(255, 0, 0), 2);//�Ԙg�l�p��`��
						cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//���S�`��

						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//����
						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font, CV_RGB(200, 0, 0));			//�ԕ���

						sprintf_s(strR, "%s\\�����摜\\Effected_%04d.bmp", FolderName, i);
						cvSaveImage(strR, src_img);
						cvShowImage("Camera", src_img);
						cvWaitKey(2);
					}
					else{
						// (2)�e���v���[�g�ɑΉ�����ʒu�ɋ�`��`��A���S�_���`��
						cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(0, 0, 255), 2);//�g�l�p��`��
						cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//���S�`��
						//�^�C���X�^���v

						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//����
						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font, CV_RGB(0, 0, 0));			//������

						num_effected++;
						sprintf_s(strR, "%s\\�����摜\\Effected_%04d.bmp", FolderName, i);
						cvSaveImage(strR, src_img);
						cvShowImage("Camera", src_img);
						cvWaitKey(2);
						test = (max_loc.x + tmp_img->width / 2);
						if (max < test){
							max = test;
							M = i;
						}
						//X,Y���W�̏o�� - for csv
						sprintf_s(XYpoint[i], ",%02d,%02d", (max_loc.x + tmp_img->width / 2), (max_loc.y + tmp_img->height / 2));
						printf("Effected_%04d.bmp �쐬 (�ގ��x%.1f%%)\n", i, max_val*100.0);

					}
				}

			}
			if (use_src_img){
				cvReleaseImage(&src_img);
			}
			cvReleaseImage(&tmp_img);
			//���ʃf�[�^�̏�������
			sprintf_s(strR, "%s\\���l�f�[�^\\���ʃf�[�^.csv", FolderName);
			if (error = fopen_s(&file, strR, "w") != 0){
				printf("%s\n", strT);
				fprintf(stderr, "���ʃf�[�^�ɏ������߂܂���.\n");
				fprintf(stderr, "�iExcel�ȂǂŊJ���Ă���Ə������߂܂���.�j\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}

			//----------------------------------------------------------------------------------------------
			//����CSV1�s�ڂ̋L�q
			//fprintf(file, "�摜�t�@�C����,��,��,�b,�~���b,�o�ߎ���,t(s),x[pixel],y[pixel],X[meter],Y[meter],,�{�[���̒��a(���[�g��)��,(�{�[���̒��a�����),�e���v���[�g�摜�ł̃s�N�Z���T�C�Y��,(�e���v���[�g�摜�̉��T�C�Y�����,1�s�N�Z�������胁�[�g���o�́�,=$N$1/$P$1\n");
			fprintf(file, "�{�[���̒��a(���[�g��)��,,,,(�����ɓ���),\n" "�e���v���[�g�摜�ł̃s�N�Z���T�C�Y��,,,,(�����ɓ���)\n" "1�s�N�Z�������胁�[�g���o�́�,,,,=$J$1/$L$1\n" "�ގ��x,%f\n" "-------,-------,-------,-------,-------,-------,-------,\n", config_val);
			fprintf(file, "�摜�t�@�C����,�o�ߎ���,t(s),x[pixel],y[pixel],X[meter],Y[meter]\n", config_val);
			if (num_effected != 0){
				//�e�s�̋L�q
				int c = 2; // line number of csv file (start from 2)
				for (i = 0; i < myFILECOUNT; i++){
					if (val4files[i] <config_val){
						//printf("%d skiped\n",i);
						//getchar();
					}
					//���l�f�[�^�Ƀt�@�C�����ȂǍ��W�A�G�N�Z����ł̌v�Z�����o�͂�����
					else{
						char FName[50];
						sprintf_s(FName, "img_%04d.bmp,%s", i, sprintClock(i));
						fprintf(file, FName);				//�t�@�C����&�����f�[�^���o��
						//fprintf(file, ",= B%d *60*60*100 + C%d * 60 *1000 + D%d * 1000 + E%d", c, c, c, c);		//�b�D�~���b���W�𐔒l�f�[�^�o��
						//fprintf(file, ",= (F%d - $F$2)/1000", c);		//�P�t���[�����Ƃ̎��ԍ��W�𐔒l�f�[�^�o��
						fprintf(file, ",= B%d - $B$2", c);		//�P�t���[�����Ƃ̎��ԍ��W�𐔒l�f�[�^�o��
						fprintf(file, XYpoint[i]);		//XY���W�𐔒l�f�[�^�o��
						//fprintf(file, ",=(H%d-$H$2)*$R$1,=(I%d-$I$2)*$R$1", c, c);		//x��y�𐔒l�f�[�^�o��
						fprintf(file, ",=(D%d-$D$2)*$N$1,=(E%d-$E$2)*$N$1", c, c);		//x��y�𐔒l�f�[�^�o��

						fprintf(file, "\n");			//���s���Ȃ��Ɖ�1��ɂȂ�
						c++;
					}
				}
			}
			fclose(file);

			printf("���l�f�[�^�o�͂��I�����܂���.\n");

			//----------------------------------------------------------------------------------------------
			fprintf(stderr, "�}�b�`���O�������I�����܂����B\n");


			printf("\n\n�ގ��x%.1f%%�ȏ�̌��o����\n�S%d���� %d�����o�ł��܂���.\n\n", config_val*100.0, num_bmp, num_effected);
			printf("---------���o�������Ȃ��ꍇ�̑Ώ�---------\n");
			printf("���e���v���[�g�摜��Config.txt��ύX���ēxMatch Template�̎��s.\n");
			cvShowImage("Camera", image1);
			D(key);


		}
		else if (key == 'R'){
			fprintf(stderr, "�}�b�`���O�̃e�X�g���s���܂�.(Space key �ŏI��).\n");
			IplImage *tmp_img;
			IplImage *tmp_img2;

			//�e���v���[�g�ǂݍ���
			sprintf_s(strR, "%s\\�e���v���[�g.bmp", FolderName);						//template picture as bmp format
			tmp_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
			if (tmp_img == NULL) {//�ǂݍ��ݎ��s
				fprintf(stderr, "�e���v���[�g�摜�̓Ǎ��݂Ɏ��s���܂���.\n");
				fprintf(stderr, "�e���v���[�g�摜���쐬���Ă�������.\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}
			tmp_img2 = cvCreateImage(cvSize(tmp_img->width, tmp_img->height), tmp_img->depth, tmp_img->nChannels);
			cvCopy(tmp_img, tmp_img2);

			//read config.txt
			if (error = fopen_s(&file_config, "./�����t�H���_/Config.txt", "r") != 0){
				fprintf(stderr, "Config.txt �̓Ǎ��݂Ɏ��s���܂���.\n");
				fprintf(stderr, "�ċN�����Ă�������.\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}
			else{
				fscanf_s(file_config, "%lf", &config_val);
				fclose(file_config);
			}

			//�������[�v����
			startClock();
			int i = 0;
			int tw, th;
			tw = tmp_img->width;
			th = tmp_img->height;
			bool q_const = TRUE;
			while (1){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space key�����͂���܂����B�L�^���I�����܂��B");
					break;
				}
				if (GetAsyncKeyState(VK_RETURN) & 0x8000){
					printf("ENTER key�����͂���܂���.");
					cvWaitKey(100);
					q_const = (!q_const);
					if (q_const){
						printf("���ǐՃ��[�h�ɕύX���܂���\n");
					}
					else{
						printf("���I�ǐՃ��[�h�ɕύX���܂���\n");
					}
				}

				cap >> frame;
				image1 = cvQueryFrame(videoCapture1);
				src_img = cvCloneImage(image1);

				saveClock(i);
				// (1)�T���摜src_img �S�̂ɑ΂��āC�e���v���[�g�̃}�b�`���O�l�i�w�肵����@�Ɉˑ��j���v�Z
				dst_size = cvSize(src_img->width - tmp_img->width + 1, src_img->height - tmp_img->height + 1);
				dst_img = cvCreateImage(dst_size, IPL_DEPTH_32F, 1);
				if (q_const){
					cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);				//���ǐ�
				}
				else{
					cvMatchTemplate(src_img, tmp_img2, dst_img, CV_TM_CCOEFF_NORMED);				//���I�ǐ�
				}
				cvMinMaxLoc(dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);
				cvReleaseImage(&dst_img);


				//���I�ǐ՗p�� tmp_img2 ��ύX
				if (!q_const){
					if (max_val < config_val){
						cvCopy(tmp_img, tmp_img2);
					}
					else{
						cvSetImageROI(src_img, cvRect(max_loc.x, max_loc.y, tw, th));
						cvCopy(src_img, tmp_img2);
						cvShowImage("template", tmp_img2);
						cvResetImageROI(src_img);
					}
				}

				// (2)�e���v���[�g�ɑΉ�����ʒu�ɋ�`��`��A���S�_���`��
				CvScalar cvrgb1 = CV_RGB(0, 0, 255);
				CvScalar cvrgb2 = CV_RGB(0, 0, 0);
				if (max_val < config_val){//�e���v���[�g�ƒT���Ώۂ̗ގ��x��config_val ����
					cvrgb1 = CV_RGB(255, 0, 0);
					cvrgb2 = CV_RGB(200, 0, 0);
				}
				cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tw, max_loc.y + th), cvrgb1, 2);//�g�l�p��`��
				cvCircle(src_img, cvPoint(max_loc.x + tw / 2, max_loc.y + th / 2), 1, CV_RGB(0, 255, 0), -1, 8);//���S�`��
				cvPutText(src_img, sprintClockwPer(i, (int)(max_val * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//����
				cvPutText(src_img, sprintClockwPer(i, (int)(max_val * 100.0)), cvPoint(20, 25), &font, cvrgb2);			//����

				cvShowImage("Camera", src_img);
				cvWaitKey(2);

				cvReleaseImage(&src_img);

			}
			cvReleaseImage(&tmp_img);
			cvReleaseImage(&tmp_img2);
			//--------------------------------------------------------------------------
			cvDestroyAllWindows();
			cvShowImage("Camera", image1);
			D(key);
		}
		//Exit-----------------------------------
		else if (key == '0' || key == 27 || key == 'q' || key == 'Q'){
			break;
		}
		else if (key == '?' || key == 'h' || key == 'H'){
			D0();
			D00();
		}
		else if (key == 'v' || key == 'V' || key == '9'){
			//for csv
			int myFILECOUNT = fscanClock();
			if (myFILECOUNT == -1){
				fprintf(stderr, "�B�e�f�[�^������܂���.\n");
				fprintf(stderr, "�B�e���s���Ă�������.\n\n");
				D('b');
				continue;
			}
			printf("\n\n%d���̎B�e�f�[�^��\�����܂�.\n", myFILECOUNT);
			//--------------------------------------------------------------------------------------
			bool use_src_img = FALSE;
			for (i = 0; i < myFILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space key�����͂���܂����B�L�^���I�����܂��B\n");
					break;
				}

				else if (key == 'V'){
					sprintf_s(strR, "%s\\�B�e�摜\\outputpic_%04d.bmp", FolderName, i);
				}
				else{
					sprintf_s(strR, "%s\\�����摜\\Effected_%04d.bmp", FolderName, i);
				}
				if (use_src_img) cvReleaseImage(&src_img);
				src_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
				//�ǂݍ��ݎ��s
				if (src_img == NULL) {
					printf("�B�e�摜�̓Ǎ��݂Ɏ��s���܂���.\n");
					continue;
				}
				use_src_img = TRUE;
				cvShowImage("Camera", src_img);
				int delay = 2;
				if (i>1){ delay = (int)((Ftime[i] - Ftime[i - 1])*1000.0); };
				printf("%s,%s\n", strR, sprintClock(i));
				cvWaitKey(delay);
			}
			if (use_src_img) cvReleaseImage(&src_img);

			cvShowImage("Camera", image1);
			D(key);
		}
	}

	cvReleaseCapture(&videoCapture1);
	cvDestroyWindow("Camera");

	return 0;
}



/*���Ԓʂ�ɓ������Ă��炤���߂̃L�[�{�[�h�����\��*/
void D(int K)		//�L�[����ꗗ�̃e�L�X�g�\��
{
	fprintf(stderr, "--------------------------------------------------------------------------\n");
	//fprintf(stderr, "��Camera�E�B���h�E���A�N�e�B�u�ɂ��邱�ƂŃL�[���삪�\\n\n");
	if (K == 'd' || K == 'D'){
		fprintf(stderr, "��͏����ŕK�v�ƂȂ�e���v���[�g���摜�̍쐬\n");
		fprintf(stderr, " b :�e���v���[�g�p�摜�̕ۑ�.\n");
		fprintf(stderr, " q or Esc : ���̃A�v���P�[�V�������I������\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else if (K == 'b' || K == 'B'){
		fprintf(stderr, "�B�e�ۑ����@\n");
		fprintf(stderr, " s :�摜��250���ۑ�\n");
		fprintf(stderr, " q or Esc : ���̃A�v���P�[�V�������I������\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else if (K == 's' || K == 't' || K == 'S'){
		fprintf(stderr, "�B�e�����s�����ꍇ�͍ĎB�e������s���Ă�������.\n");
		fprintf(stderr, " s :�摜��250���ۑ�\n");


		fprintf(stderr, " �B�e���������ꍇ�͎��̓�����s���Ă�������.\n");
		fprintf(stderr, " ��͏����A�I������\n");
		fprintf(stderr, " m :Match Template�@�̎��s,���l�o��.\n");
		fprintf(stderr, " q or Esc : ���̃A�v���P�[�V�������I������\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else if (K == 'm' || K == 'M'){
		fprintf(stderr, " ��͏������Ď��s�A�I������\n");
		fprintf(stderr, " m :Match Template�@�̎��s,���l�o��.\n");
		fprintf(stderr, " q or Esc : ���̃A�v���P�[�V�������I������\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else{
		fprintf(stderr, "�L�[����(%c)�̏������I���܂���.���̏������s���Ă��������D\n", K);
		D0();
	}

}


/*

���̎����\�t�g�� OpenCV-2.4.9 ���g���Ă��܂��B���ЊJ���ɎQ�����Ă��������B

OpenCV��BSD���C���Z���X�Ɋ�Â��A���p/�񏤗p��킸�A
�����Ŏg�p���鎖���\�ł��B�ȉ��ɁA OpenCV ��
���C�Z���X��\�����܂��B (opencv-2.4.9\sources\LICENCE ���S��)


By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


License Agreement
For Open Source Computer Vision Library
(3-clause BSD License)

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the names of the copyright holders nor the names of the contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.


*/