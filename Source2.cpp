// Source2.0 version 2017-09-04
#include <stdio.h>
#include <assert.h>
#include <tchar.h>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>
#include <direct.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <conio.h> // for windows only kbhit and getch
#include <time.h> //時間出力用
#include <windows.h>
/*OpenCV 2.4.9のライブラリ*/
#ifdef _DEBUG
#pragma comment (lib,    "opencv_core249d.lib")
#pragma comment (lib, "opencv_imgproc249d.lib")
#pragma comment (lib, "opencv_highgui249d.lib")
#pragma comment (lib, "opencv_video249d.lib")
#else
#pragma comment (lib,    "opencv_core249.lib")
#pragma comment (lib, "opencv_imgproc249.lib")
#pragma comment (lib, "opencv_highgui249.lib")
#pragma comment (lib, "opencv_video249.lib")
#endif

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib,"user32.lib")

#define COUNT 1000 // 特徴点の個数
#define FILECOUNT		250
#define FILECOUNT_MAX   9999
//100枚で3秒程度	(30fps)	//これを小さくし過ぎるとプログラムで使用する配列のサイズが変わる
//バッファサイズが足りず実行できないことがある

/* グローバル変数 */
//	IplImage *img = 0;
const char* FolderName = "実験フォルダ";//フォルダ名
errno_t error;
using namespace std;
/*最初のキーボード説明表示*/
void D0(void)		//キー操作一覧のテキスト表示
{
	fprintf(stderr,
		"--------------------------------------------------------------------------\n"
		" 1 :テンプレート用画像の保存.\n"
		" 2 :テンプレート用画像の編集・トリミング.\n"
		" 3 :画像を250枚保存(Spece Keyの入力で終了).\n"
		" 4 :テンプレートマッチングの実行,結果データ出力.\n"
		" 5 :結果データの解析.\n"
		" 6 :データ保存.\n"
		" 0 or q or Esc : このアプリケーションを終了する\n"
		" 7 :マッチングの類似度を変更する\n"
		" 8 :データコピーのためフォルダを開く\n"
		" c :カメラを変更する\n"
		" H :ヘルプ表示\n"
		" 実験フォルダがあることを確認して、撮影・解析を行ってください.\n"
		"-------------------------------------------------------------------------\n"
		);
}
void D00(void)		//キー操作一覧のテキスト表示
{
	fprintf(stderr,
		" 0 or q or Esc : このアプリケーションを終了する\n"
		" 9 :処理画像を表示する\n"
		" b :背景用画像の保存.\n"
		" C :相互相関係数分布の表示\n"
		" d :実験フォルダの作成.\n"
		" M :テンプレートマッチ（バグ有）\n"
		" R :テンプレートの練習\n"
		" S :画像を250枚直接ハードディスクに保存\n"
		" t :画像を無制限にメモリに保存(Spece Keyの入力で終了)\n"
		" T :画像を無制限に直接ハードディスクに保存\n"
		" V :撮影画像を表示する\n"
		" @ :様々な画像処理を行う(edge,rgb,color)\n"
		"-------------------------------------------------------------------------\n\n"
		);
}
void D(int K)
{
	/*順番通りに動かしてもらうためのキーボード説明表示を入れてもよい*/
	D0();
}

//色検出範囲設定用関数
int imgavemin(int ave, int n){
	int ave1;
	if (ave - n < 0){
		ave1 = 0;
	}
	else{
		ave1 = ave - n;
	}
	return ave1;
}
int imgavemax(int ave, int n){
	int ave2;
	if (ave + n > 255){
		ave2 = 255;
	}
	else{
		ave2 = ave + n;
	}
	return ave2;
}
int imgaveh1(int ave, int n){
	int aveh;
	if (ave - n < 0){
		aveh = 180 + ave - n;
	}
	else{
		aveh = ave - n;
	}
	return aveh;
}
int imgaveh2(int ave, int n){
	int aveh;
	if (ave + n > 180){
		aveh = ave + n - 180;
	}
	else{
		aveh = ave + n;
	}
	return aveh;
}

//時刻取得関数 windows 依存
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
	sprintf_s(strS, "%s\\数値データ\\時間データ.csv", FolderName);
	//時間数値データの作成
	if ((error = fopen_s(&file_s, strS, "w")) != 0) {
		printf("error\n");
	}
	for (int j = 0; j < Ftime_max; j++){
		fprintf(file_s, "%lf\n", Ftime[j]);		//時間を数値データに出力
	}
	fclose(file_s);
	fprintf(stderr, "時間データを出力しました.\n");

}
int fscanClock(void){
	FILE *file_csv;
	char strT[_MAX_PATH] = "";
	sprintf_s(strT, "%s\\数値データ\\時間データ.csv", FolderName);
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

#ifdef USE_OLD_VERSION__
char winTime[FILECOUNT][20];//ファイル時刻用文字列
int fscanClock0(void){
	FILE *file_csv;
	char strT[_MAX_PATH] = "";
	sprintf_s(strT, "%s\\数値データ\\時間データ.csv", FolderName);
	if (error = fopen_s(&file_csv, strT, "r") != 0){
		printf("%s\n", strT);
		return(-1);
	}
	for (int i = 0; i < FILECOUNT; i++){
		fscanf_s(file_csv, "%s", &winTime[i]);
	}
	fclose(file_csv);
	return(FILECOUNT);
}
char* sprintClock0(int i){
	sprintf_s(charTimes, "%s", winTime[i]);
	return(charTimes);
}
void fprintClock0(void){
	char strS[_MAX_PATH] = "";
	FILE* file_s;
	sprintf_s(strS, "%s\\数値データ\\時間データ.csv", FolderName);
	//時間数値データの作成
	if ((error = fopen_s(&file_s, strS, "w")) != 0) {
		printf("error\n");
	}

	//各行の記述
	for (int j = 0; j < FILECOUNT; j++){
		//テキスト作成
		fprintf(file_s, winTime[j]);		//時間を数値データに出力
		fprintf(file_s, "\n");			//改行しないと横1列になる
	}
	fclose(file_s);
	fprintf(stderr, "時間データを出力しました.\n");
}
void saveClock0(int i){
	SYSTEMTIME stime;
	GetLocalTime(&stime);
	//MSに取得時間を代入
	sprintf_s(winTime[i], "%02d,%02d,%02d,%03d",
		stime.wHour,
		stime.wMinute,
		stime.wSecond,
		stime.wMilliseconds);
}
#endif

int main(int argc, char **argv)
{
	int i, test, max = 0, M;
	int back_R = 0, back_G = 0, back_B = 0;
	int flag_Temp = 0;
	int Savecount = -1;//判断処理用変数
	double min_val, max_val;
	double Cmax, Cmin;
	double config_val;//Config.txt から読み込む類似度
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

	//数値データファイル入出力
	FILE *file, *file_config;

	//フォント - time stamp
	CvFont font, font_w;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 1.0, 0, 1, CV_AA);
	cvInitFont(&font_w, CV_FONT_HERSHEY_SIMPLEX, 0.5, 1.0, 0, 3, CV_AA);

	int flagCam = 0;
	int iCAM = 0;
	int defaultCAM = 0;// ThinkPad カメラ用の設定
	CvCapture * videoCaptureCAM;
	//defaultCAMの値設定
	for (iCAM = defaultCAM; iCAM >= 0; iCAM--){
		videoCaptureCAM = cvCaptureFromCAM(iCAM);
		if (flagCam == 0){
			if (!(videoCaptureCAM == NULL)){
				defaultCAM = iCAM;
				flagCam = 1;
			}
		}
	}
	CvCapture * videoCapture1 = cvCaptureFromCAM(defaultCAM);
	//１台のみの場合cvCaptureFromCAMの引数はなんでもいい
	//複数台の場合はPC起動時の接続順が引数になる

	D0();
	if (_mkdir("実験フォルダ") == 0){//.exeと同じ階層にディレクトリを作成
		printf("フォルダ作成\n");
	}
	else{
		printf("フォルダ作成に失敗しました。既にフォルダが存在する可能性があります。\n");
	}
	_mkdir("実験フォルダ\\数値データ");
	_mkdir("実験フォルダ\\撮影画像");
	_mkdir("実験フォルダ\\処理画像");

	if (error = fopen_s(&file_config, "./実験フォルダ/Config.txt", "r") != 0){
		fopen_s(&file_config, "./実験フォルダ/Config.txt", "w");
		fprintf(file_config, "0.70");
		config_val = 0.70;
		fclose(file_config);
	}
	else{
		fscanf_s(file_config, "%lf", &config_val);
		fclose(file_config);
		fprintf(stderr, "\n類似度%.1f%%に設定しました.\n", config_val*100.0);
	}
	fprintf(stderr, "\n実験フォルダ作成し開きます.\n");


	//カメラを開く-----------------
	cv::VideoCapture cap;
	cap.open(0);

	if (!cap.isOpened())	{
		printf("\n\n\n\nエラー：カメラを認識できませんでした.\n終了します.\n\n");
		getchar();
		cv::waitKey(10000);
		return -1;
	}

	//相互相関係数分布を生成するための変数
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

		//ウィンドウ生成------------------------------------------------------------
		cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE);
		IplImage *image1 = cvQueryFrame(videoCapture1);
		cvShowImage("Camera", image1);

		//実験基本動作ここから-----------------------------------------------------------------------------
		//画像の1枚の保存を行う------------------------------------------------------------------------------
		if (key == '1'){
			//	IplImage output = frame;
			IplImage *output = cvQueryFrame(videoCapture1);

			sprintf_s(strB, "%s\\テンプレート.bmp", FolderName);
			cvSaveImage(strB, output);

			fprintf(stderr, "テンプレート用画像の保存に成功しました\n");
			D(key);
		}
		//ペイントで対象(テンプレート用画像)を開く-------------------------------------------
		if (key == '2'){
			system("mspaint \"実験フォルダ\\テンプレート.bmp");
			D(key);
		}
		//設定枚数分の画像をメモリ使用し保存する---------------------------------------------
		if (key == '3'){
			fprintf(stderr, "画像をメモリに格納します.(Space key で終了).\n");
			cvDestroyAllWindows();
			IplImage *output;
			IplImage** vout;
			vout = (IplImage **)malloc(sizeof(IplImage *)*FILECOUNT);
			int i;
			startClock();
			//for (i = 0; i < 3000; i++){
			for (i = 0; i < FILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space keyが入力されました。記録を終了します。");
					break;
				}

				cap >> frame;
				output = cvQueryFrame(videoCapture1);
				vout[i] = cvCloneImage(output);
				saveClock(i);
				printf("%04d (Space key で終了)\n", i);
			}
			int imax = i;
			fprintf(stderr, "画像をディスクに保存します.\n");

			for (int i = 0; i < imax; i++){
				sprintf_s(strS, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
				if (vout[i] == NULL) continue;
				cvShowImage("Camera", vout[i]);
				cvWaitKey(1);
				cvSaveImage(strS, vout[i]);
				printf("outputpic_%04d.bmp 保存\n", i);
				cvReleaseImage(&(vout[i]));
			}
			free(vout);

			fprintf(stderr, "撮影画像を保存しました.\n");
			fprintClock();

			cvShowImage("Camera", image1);
			D(key);
		}
		//テンプレートマッチ-------------------------------------------------
		if (key == '4'){
			int num_bmp = 0;
			int num_effected = 0;
			IplImage *tmp_img;
			double val4files[FILECOUNT_MAX];//類似度
			char XYpoint[FILECOUNT_MAX][20];//座標取得の文字列20文字
			for (int i1 = 0; i1 < FILECOUNT_MAX; i1++){
				val4files[i1] = -0.01;
			}
			flag_Temp = 0;

			//----------------------------------------------------------------------
			printf("\n\n解析処理を行います.\n");
			// precheck for 結果データ
			sprintf_s(strR, "%s\\数値データ\\結果データ.csv", FolderName);
			if (error = fopen_s(&file, strR, "w") != 0){
				printf("%s\n", strR);
				fprintf(stderr,
					"結果データに書き込めません.処理を中断します.\n"
					"（Excelなどで開いていると書き込めませんので閉じてください.）\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}
			else{ fclose(file); }

			//for csv
			int myFILECOUNT = fscanClock();
			if (myFILECOUNT == -1){
				fprintf(stderr,
					"撮影画像データがありません.\n"
					"手順'3'を行ってください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}
			printf("%d個のテンプレートマッチング処理を行います。少々お待ちください。\n", myFILECOUNT);
			printf("(Space key で中断)\n");

			//読み込み
			sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);//template picture as bmp format
			tmp_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

			//読み込み失敗
			if (tmp_img == NULL) {
				fprintf(stderr,
					"テンプレート画像の読込みに失敗しました.\n"
					"手順'1'を行ってください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}

			//read config.txt
			if (error = fopen_s(&file_config, "./実験フォルダ/Config.txt", "r") != 0){
				fprintf(stderr,
					"Config.txt の読込みに失敗しました.\n"
					"再起動(手順'0')してください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}
			else{
				fscanf_s(file_config, "%lf", &config_val);
				fclose(file_config);
			}
			bool use_src_img = FALSE;
			for (i = 0; i < myFILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space keyが入力されました。テンプレートマッチング処理を中断します.\n");
					flag_Temp = 1;
					break;
				}
				sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
				if (use_src_img){
					cvReleaseImage(&src_img);
				}
				src_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
				use_src_img = TRUE;

				//読み込み失敗
				if (src_img == NULL) {
					printf("撮影画像の読込みに失敗しました.\n");
				}
				else{
					num_bmp++;
					printf("outputpic_%04d.bmp:", i);
					// (1)探索画像全体に対して，テンプレートのマッチング値（指定した手法に依存）を計算
					dst_size = cvSize(src_img->width - tmp_img->width + 1, src_img->height - tmp_img->height + 1);
					dst_img = cvCreateImage(dst_size, IPL_DEPTH_32F, 1);
					/*テンプレートマッチングのmethod(計算方法)の種類
						CV_TM_SQDIFF		エラー発生
						CV_TM_CCORR			エラー発生
						CV_TM_CCOEFF		Correlation coefficient　相関係数
						CV_TM_SQDIFF_NORMED	SSD(Sum of Squared Difference)　輝度差の二乗和
						CV_TM_CCORR_NORMED	NCC（Normalized Cross-Correlation）　相互相関の正規化
						CV_TM_CCOEFF_NORMED	ZNCC（Zero-mean Normalized Cross-Correlation）相関係数の正規化
						*/
					cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);//methodも変えつつ検証が必要

					cvMinMaxLoc(dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);

					/*150番目の類似度確認用*/
					if (i == 150){
						img_ccoeff = dst_img;
						cvMinMaxLoc(img_ccoeff, &Cmin, &Cmax, &Pmin, &Pmax, NULL);

					}
					cvReleaseImage(&dst_img);
					val4files[i] = max_val;

					if (val4files[i] < config_val){//テンプレートと探索対象の類似度がconfig_val 未満
						//X,Y座標の出力 - for csv
						sprintf_s(XYpoint[i], ",%02d,%02d", 0, 0);
						printf("類似度%.1f(%.1f%%未満)\n", max_val*100.0, config_val*100.0);

						cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(255, 0, 0), 2);//赤枠四角を描画
						cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画

						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//白縁
						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font, CV_RGB(200, 0, 0));			//赤文字

						sprintf_s(strR, "%s\\処理画像\\Effected_%04d.bmp", FolderName, i);
						cvSaveImage(strR, src_img);
						cvShowImage("Camera", src_img);
						cvWaitKey(1);
					}
					else{
						// テンプレートに対応する位置に矩形を描画、中心点も描画
						cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(0, 0, 255), 2);//青枠四角を描画
						cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画
						//タイムスタンプ

						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//白縁
						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font, CV_RGB(0, 0, 0));			//黒文字

						num_effected++;
						sprintf_s(strR, "%s\\処理画像\\Effected_%04d.bmp", FolderName, i);
						cvSaveImage(strR, src_img);
						cvShowImage("Camera", src_img);
						cvWaitKey(1);
						test = (max_loc.x + tmp_img->width / 2);
						if (max < test){
							max = test;
							M = i;
						}
						//X,Y座標の出力 - for csv
						sprintf_s(XYpoint[i], ",%02d,%02d", (max_loc.x + tmp_img->width / 2), (max_loc.y + tmp_img->height / 2));
						printf("Effected_%04d.bmp 作成 (類似度%.1f%%)\n", i, max_val*100.0);

					}
				}

			}

			if (flag_Temp == 0){
				if (use_src_img){
					cvReleaseImage(&src_img);
				}
				cvReleaseImage(&tmp_img);
				fprintf(stderr,
					"マッチング処理が終了しました。\n"
					"結果データをCSVファイルに書き込みます。\n"
					);

				//テンプレートマッチの結果データの書き込み
				sprintf_s(strR, "%s\\数値データ\\結果データ.csv", FolderName);
				if (error = fopen_s(&file, strR, "w") != 0){
					printf("%s\n", strT);
					fprintf(stderr,
						"結果データに書き込めません.\n"
						"（Excelなどで開いていると書き込めません.）\n\n"
						);
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}

				//結果CSV1行目の記述
				fprintf(file, "画像ファイル名,t[s],x[pixel],y[pixel],X[meter],Y[meter],類似度,,←１ピクセルが何メートルか？\n");

				if (num_effected != 0){
					//各行の記述
					int c = 2; // line number of csv file (start from 2)
					int flagcsv = 0;
					double elatime, elatime0;
					char *ale;
					for (i = 0; i < myFILECOUNT; i++){
						if (val4files[i] < config_val){
							//printf("%d skiped\n",i);
							//getchar();
						}
						//数値データにファイル名など座標、エクセル上での計算式を出力させる
						else{
							if (flagcsv == 0){
								elatime0 = strtod(sprintClock(i), &ale);
								flagcsv = 1;
							}
							char FName[50];
							sprintf_s(FName, "img_%04d.bmp", i);
							fprintf(file, FName);	//ファイル名&時刻データを出力
							elatime = strtod(sprintClock(i), &ale);
							elatime -= elatime0;
							fprintf(file, ",%.6lf", elatime);	//１フレームごとの時間座標を数値データ出力
							fprintf(file, XYpoint[i]);	//XY座標を数値データ出力
							fprintf(file, ",=(C%d-$C$2)*$H$1,=(D%d-$D$2)*$H$1", c, c);	//xとyを数値データ出力
							fprintf(file, ",%.6lf", val4files[i]);
							fprintf(file, "\n");	//改行しないと横1列になる
							c++;
						}
					}
				}
				fclose(file);

				printf("数値データ出力が終了しました.\n");

				printf("\n\n類似度%.1f%%以上の検出結果\n全%d枚中 %d枚検出できました.\n\n", config_val*100.0, num_bmp, num_effected);
				printf("---------検出数が少ない場合の対処---------\n");
				printf("※テンプレート画像やConfig.txtを変更し再度Match Templateの実行.\n");
			}
			cvShowImage("Camera", image1);
			D(key);
			key = 32;
		}
		if (key == '5'){
			system("explorer \"実験フォルダ\\数値データ\\結果データ.csv");		//エクスプローラーで対象を開く
			D(key);
		}
		else if (key == '6'){
			system("xcopy /S /C /I /Y \".\\実験フォルダ\" %date:~-5,2%%date:~-2%%time:~0,2%%time:~3,2%%time:~6,2%");
			D(key);
		}
		//実験基本動作ここまで-----------------------------------------------------------------------------




		//実験補助動作ここから-----------------------------------------------------------------------------
		//カメラ切り替え----------------------------------
		if (key == 'c'){
			videoCaptureCAM;
			flagCam = 0;
			defaultCAM--;
			if (defaultCAM < 0){
				defaultCAM = 3;
			}
			for (iCAM = defaultCAM; iCAM >= 0; iCAM--){
				videoCaptureCAM = cvCaptureFromCAM(iCAM);
				if (flagCam == 0){
					if (!(videoCaptureCAM == NULL)){
						defaultCAM = iCAM;
						flagCam = 1;
					}
				}
			}

			printf("\n\nカメラ番号を%02dへ変更しました.\n画面の動きがあればOKです\n", defaultCAM);
			cvReleaseCapture(&videoCapture1);
			videoCapture1 = cvCaptureFromCAM(defaultCAM);
			D(key);
		}
		//類似度変更----------------------------------
		if (key == '7'){
			char aa[10];
			int numa = 0;
			printf("類似度の数値（1～99）を入力してください\n変更しない場合は'N'");
			printf(" 現在値 : %.1lf%%\n　参考（高い90, 普通75, 低い60）\n", config_val * 100);
			scanf_s("%s", aa, 10);
			const std::string stra(aa);
			if (!strcmp(aa, "N")){
			}
			else{
				sscanf_s(stra.c_str(), "%d", &numa);
				if (numa >= 0 && numa <= 100){
					config_val = numa / 100.0;
					errno_t err = fopen_s(&file_config, "./実験フォルダ/Config.txt", "w+");
					if (err)
						printf_s("The file ./実験フォルダ/Config.txt was not opened\n");
					else
					{
						double cva = config_val;
						fprintf_s(file_config, "%lf", cva);// Set pointer to beginning of file:  
						fseek(file_config, 0L, SEEK_SET);
						fscanf_s(file_config, "%lf", &config_val);
						fclose(file_config);
					}
					printf("\n類似度を%.1lf%%に変更しました\n", config_val * 100);
				}
				else{
					printf("\n類似度が変更されませんでした\n");
				}
			}
			D(key);
			key = 32;
		}
		if (key == '8'){
			system("explorer \"実験フォルダ");//エクスプローラーで対象を開く
			D(key);
		}
		//相互相関係数分布
		if (key == 'C'){
			cvConvertScale(img_ccoeff, img_ccoeff, 1.0 / Cmax, 0.0);
			cvShowImage("相互相関係数分布", img_ccoeff);
		}
		//ディレクトリ作成-----------------------------------
		if (key == 'd' || key == 'D'){ //C:\\Users\\yama\\Documents\\実験フォルダ-webCam
			if (_mkdir("実験フォルダ") == 0){//.exeと同じ階層にディレクトリを作成
				printf("フォルダ作成\n");
			}
			else{
				printf("フォルダ作成に失敗しました。既にフォルダが存在する可能性があります。\n");
			}
			system("explorer \"実験フォルダ");//エクスプローラーで対象を開く
			_mkdir("実験フォルダ\\数値データ");
			_mkdir("実験フォルダ\\撮影画像");
			_mkdir("実験フォルダ\\処理画像");

			fprintf(stderr, "実験用フォルダ作成を開きます.\n");
			D(key);
		}
		//設定枚数分の画像をメモリ使用し無限に保存する---------------
		if (key == 't'){
			fprintf(stderr, "画像メモリ格納開始.(Space key で終了).\n");
			cvDestroyAllWindows();
			IplImage *output;
			IplImage** vout;
			int mFILECOUNT = FILECOUNT_MAX;
			vout = (IplImage **)malloc(sizeof(IplImage *)*mFILECOUNT);
			int i;
			startClock();
			for (i = 0; i < mFILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space keyが入力されました。記録を終了します。");
					break;
				}
				cap >> frame;
				//	IplImage output = frame;
				output = cvQueryFrame(videoCapture1);
				vout[i] = cvCloneImage(output);
				if (vout[i] == NULL){
					printf("メモリ格納に失敗しました。記録を終了します。");
					break;
				}
				saveClock(i);
				printf("%04d メモリ格納\n", i);
			}
			int imax = i;
			fprintf(stderr, "画像保存開始.\n");

			for (int i = 0; i < imax; i++){
				sprintf_s(strS, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
				if (vout[i] == NULL) continue;
				cvSaveImage(strS, vout[i]);
				printf("outputpic_%04d.bmp 保存\n", i);
				cvReleaseImage(&(vout[i]));
			}
			free(vout);

			fprintf(stderr, "撮影画像を保存しました.\n");
			fprintClock();
			cvShowImage("Camera", image1);
			D(key);
		}
		//背景画像を撮影する-----------------------------------
		if (key == 'b'){
			IplImage *output = cvQueryFrame(videoCapture1);

			sprintf_s(strB, "%s\\背景.bmp", FolderName);
			cvSaveImage(strB, output);

			fprintf(stderr, "背景用画像の保存に成功しました\n");
			D(key);
		}
		//設定枚数分の画像を直接HDに保存する------------------------------------------------------------------------
		if (key == 'S'){
			fprintf(stderr, "画像保存開始.\n");
			cvDestroyAllWindows();
			startClock();
			for (int i = 0; i < FILECOUNT; i++){
				cap >> frame;
				IplImage *output = cvQueryFrame(videoCapture1);

				sprintf_s(strS, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
				saveClock(i);
				cvSaveImage(strS, output);

				printf("outputpic_%04d.bmp 保存\n", i);
			}
			fprintf(stderr, "撮影画像を保存しました.\n");
			fprintClock();

			cvShowImage("Camera", image1);
			D(key);
		}
		//無制限に保存する--------------------HDD領域が許す限り記録できるはず
		if (key == 'T'){
			//保存中Cameraウィンドウが応答中になるのでウィンドウを消しておく
			cvDestroyAllWindows();

			Savecount = 0;
			fprintf(stderr, "無制限に保存する Space keyで終了");
			startClock();
			for (int i = 0; i < FILECOUNT_MAX; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space keyが入力されました。記録を終了します。");
					break;
				}

				cap >> frame;
				//IplImage output = frame;
				IplImage *output = cvQueryFrame(videoCapture1);

				sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
				saveClock(i);
				cvSaveImage(strR, output);
				printf("outputpic_%04d.bmp :", i);
				i++;
			}

			//Cameraウィンドウ再表示
			cvShowImage("Camera", image1);
			fprintClock();
			fprintf(stderr, "画像の保存に成功しました!\n");
			D(key);
		}
		//マッチテンプレート old version(メモリリークあり！！！）
		if (key == 'M'){
			int num_bmp = 0;
			int num_effected = 0;
			IplImage *tmp_img;

			double val4files[FILECOUNT];//類似度
			char XYpoint[FILECOUNT][20];//座標取得の文字列20文字
			for (int i1 = 0; i1 < FILECOUNT; i1++){
				val4files[i1] = -0.01;
			}

			cvDestroyAllWindows();
			printf("\n\n解析処理を行います.\n");// precheck for 結果データ
			sprintf_s(strR, "%s\\数値データ\\結果データ.csv", FolderName);
			if (error = fopen_s(&file, strR, "w") != 0){
				printf("%s\n", strR);
				fprintf(stderr,
					"結果データに書き込めません.処理を中断します.\n"
					"（Excelなどで開いていると書き込めませんので閉じてください.）\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}
			else{ fclose(file); }

			//for csv
			if (fscanClock() == -1){
				fprintf(stderr,
					"撮影データがありません.\n"
					"撮影を行ってください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}
			printf("テンプレートマッチング処理を行います。少々お待ちください。\n");

			//読み込み
			sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);//template picture as bmp format
			tmp_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

			//読み込み失敗
			if (tmp_img == NULL) {
				fprintf(stderr,
					"テンプレート画像の読込みに失敗しました.\n"
					"テンプレート画像を作成してください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}

			//read config.txt
			if (error = fopen_s(&file_config, "./実験フォルダ/Config.txt", "r") != 0){
				fprintf(stderr,
					"Config.txt の読込みに失敗しました.\n"
					"再起動してください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;

			}
			else{
				fscanf_s(file_config, "%lf", &config_val);
				fclose(file_config);
			}

			for (i = 0; i < FILECOUNT; i++){
				sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
				src_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

				//読み込み失敗
				if (src_img == NULL) {
					fprintf(stderr,
						"撮影画像の読込みに失敗しました.\n"
						"撮影画像をやり直してください.\n\n"
						);
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}
				else{
					num_bmp++;
					printf("outputpic_%04d.bmp:", i);
					// (1)探索画像全体に対して，テンプレートのマッチング値（指定した手法に依存）を計算
					dst_size = cvSize(src_img->width - tmp_img->width + 1, src_img->height - tmp_img->height + 1);
					dst_img = cvCreateImage(dst_size, IPL_DEPTH_32F, 1);
					cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);//methodも変えつつ検証が必要

					cvMinMaxLoc(dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);

					//150番目の類似度確認用
					if (i == 150){
						img_ccoeff = dst_img;
						cvMinMaxLoc(img_ccoeff, &Cmin, &Cmax, &Pmin, &Pmax, NULL);
					}
					cvReleaseImage(&dst_img);
					max_val = max_val;
					//テンプレートと探索対象の類似度がconfig_val 未満は捨てる
					if (val4files[i] < config_val){
						//X,Y座標の出力 - for csv
						sprintf_s(XYpoint[i], ",%02d,%02d", 0, 0);
						printf("類似度%.1f(%.1f%%未満)\n", max_val*100.0, config_val*100.0);
					}
					else{
						//テンプレートに対応する位置に矩形を描画、中心点も描画
						cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(255, 0, 0), 2);//四角を描画
						cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画
						//タイムスタンプ

						cvPutText(src_img, sprintClock(i), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//白縁
						cvPutText(src_img, sprintClock(i), cvPoint(20, 25), &font, CV_RGB(0, 0, 0));			//黒文字

						num_effected++;
						sprintf_s(strR, "%s\\処理画像\\Effected_%04d.bmp", FolderName, i);
						cvSaveImage(strR, src_img);
						cvReleaseImage(&src_img);
						test = (max_loc.x + tmp_img->width / 2);
						if (max < test){
							max = test;
							M = i;
						}
						//X,Y座標の出力 - for csv
						sprintf_s(XYpoint[i], ",%02d,%02d", (max_loc.x + tmp_img->width / 2), (max_loc.y + tmp_img->height / 2));
						printf("Effected_%04d.bmp 作成 (類似度%.1f%%)\n", i, max_val*100.0);
					}
				}
			}
			cvReleaseImage(&tmp_img);

			fprintf(stderr,
				"マッチング処理が終了しました。\n"
				"結果データをCSVファイルに書き込みます。\n"
				);

			//結果データの書き込み
			sprintf_s(strR, "%s\\数値データ\\結果データ.csv", FolderName);
			if (error = fopen_s(&file, strR, "w") != 0){
				printf("%s\n", strT);
				fprintf(stderr,
					"結果データに書き込めません.\n"
					"（Excelなどで開いていると書き込めません.）\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}

			if (num_effected != 0){
				//結果CSV1行目の記述
				fprintf(file, "画像ファイル名,t[s],x[pixel],y[pixel],X[meter],Y[meter],類似度,,←１ピクセルが何メートルか？\n");

				//各行の記述
				int c = 2; //line number of csv file (start from 2)
				int flagcsv = 0;
				double elatime, elatime0;
				char *ale;
				for (i = 0; i < FILECOUNT; i++){
					if (val4files[i] < config_val){
						//printf("%d skiped\n",i);
						//getchar();
					}
					//数値データにファイル名など座標、エクセル上での計算式を出力させる
					else{
						if (flagcsv == 0){
							elatime0 = strtod(sprintClock(i), &ale);
							flagcsv = 1;
						}
						char FName[50];
						sprintf_s(FName, "img_%04d.bmp", i);
						fprintf(file, FName);	//ファイル名&時刻データを出力
						elatime = strtod(sprintClock(i), &ale);
						elatime -= elatime0;
						fprintf(file, ",%.6lf", elatime);		//１フレームごとの時間座標を数値データ出力
						fprintf(file, XYpoint[i]);		//XY座標を数値データ出力
						fprintf(file, ",=(C%d-$C$2)*$H$1,=(D%d-$D$2)*$H$1", c, c);	//xとyを数値データ出力
						fprintf(file, ",%.6lf", val4files[i]);
						fprintf(file, "\n");			//改行しないと横1列になる
						c++;
					}
				}
				printf("数値データ出力が終了しました.\n");

				printf("\n\n類似度%.1f%%以上の検出結果\n全%d枚中 %d枚検出できました.\n\n", config_val*100.0, num_bmp, num_effected);
				printf("---------検出数が少ない場合の対処---------\n");
				printf("※テンプレート画像やConfig.txtを変更し再度Match Templateの実行.\n");
				cvShowImage("Camera", image1);
				D(key);
			}
			fclose(file);
		}
		if (key == 'R'){
			fprintf(stderr, "マッチングのテストを行います.(Space key で終了。ENTERでモード変更).\n");
			IplImage *tmp_img;
			IplImage *tmp_img2;

			//テンプレート読み込み
			sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);						//template picture as bmp format
			tmp_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
			if (tmp_img == NULL) {//読み込み失敗
				fprintf(stderr,
					"テンプレート画像の読込みに失敗しました.\n"
					"テンプレート画像を作成してください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}
			tmp_img2 = cvCreateImage(cvSize(tmp_img->width, tmp_img->height), tmp_img->depth, tmp_img->nChannels);
			cvCopy(tmp_img, tmp_img2);

			//read config.txt
			if (error = fopen_s(&file_config, "./実験フォルダ/Config.txt", "r") != 0){
				fprintf(stderr,
					"Config.txt の読込みに失敗しました.\n"
					"再起動してください.\n\n"
					);
				cvShowImage("Camera", image1);
				D(key);
				continue;
			}
			else{
				fscanf_s(file_config, "%lf", &config_val);
				fclose(file_config);
			}
			//無限ループ処理
			startClock();
			int i = 0;
			int tw, th;
			tw = tmp_img->width;
			th = tmp_img->height;
			bool q_const = TRUE;
			while (1){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space keyが入力されました。記録を終了します。");
					break;
				}
				if (GetAsyncKeyState(VK_RETURN) & 0x8000){
					printf("ENTER keyが入力されました.");
					cvWaitKey(100);
					q_const = (!q_const);
					if (q_const){
						printf("定常追跡モードに変更しました\n");
					}
					else{
						printf("動的追跡モードに変更しました\n");
					}
				}

				cap >> frame;
				image1 = cvQueryFrame(videoCapture1);
				src_img = cvCloneImage(image1);

				saveClock(i);
				// (1)探索画像src_img 全体に対して，テンプレートのマッチング値（指定した手法に依存）を計算
				dst_size = cvSize(src_img->width - tmp_img->width + 1, src_img->height - tmp_img->height + 1);
				dst_img = cvCreateImage(dst_size, IPL_DEPTH_32F, 1);
				if (q_const){
					cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);	//定常追跡
				}
				else{
					cvMatchTemplate(src_img, tmp_img2, dst_img, CV_TM_CCOEFF_NORMED);	//動的追跡
				}
				cvMinMaxLoc(dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);
				cvReleaseImage(&dst_img);


				//動的追跡用に tmp_img2 を変更
				if (max_val < config_val){
					cvCopy(tmp_img, tmp_img2);
				}
				else{
					cvSetImageROI(src_img, cvRect(max_loc.x, max_loc.y, tw, th));
					cvCopy(src_img, tmp_img2);
					cvShowImage("temp", tmp_img2);
					cvResetImageROI(src_img);
				}

				// (2)テンプレートに対応する位置に矩形を描画、中心点も描画
				CvScalar cvrgb1 = CV_RGB(0, 0, 255);
				CvScalar cvrgb2 = CV_RGB(0, 0, 0);
				if (max_val < config_val){//テンプレートと探索対象の類似度がconfig_val 未満
					cvrgb1 = CV_RGB(255, 0, 0);
					cvrgb2 = CV_RGB(200, 0, 0);
				}
				cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tw, max_loc.y + th), cvrgb1, 2);//枠四角を描画
				cvCircle(src_img, cvPoint(max_loc.x + tw / 2, max_loc.y + th / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画
				cvPutText(src_img, sprintClockwPer(i, (int)(max_val * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));//白縁
				cvPutText(src_img, sprintClockwPer(i, (int)(max_val * 100.0)), cvPoint(20, 25), &font, cvrgb2);//文字

				cvShowImage("Camera", src_img);
				cvWaitKey(2);

				cvReleaseImage(&src_img);
			}
			cvReleaseImage(&tmp_img);
			cvReleaseImage(&tmp_img2);

			cvShowImage("Camera", image1);
			D(key);
		}

		//Exit-----------------------------------
		if (key == '0' || key == 27 || key == 'q' || key == 'Q'){
			break;
		}
		//Help-----------------------------------
		if (key == '?' || key == 'h' || key == 'H'){
			D0();
			D00();
		}
		if (key == 'V' || key == '9'){
			//---------------------------------------
			//for csv
			int myFILECOUNT = fscanClock();
			if (myFILECOUNT == -1){
				fprintf(stderr, "撮影データがありません.\n");
				fprintf(stderr, "撮影を行ってください.\n\n");
				D(key);
				continue;
			}
			printf("\n\n%d枚の撮影データを表示します.\n", myFILECOUNT);
			//--------------------------------------------------------------------------------------
			bool use_src_img = FALSE;
			for (i = 0; i < myFILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space keyが入力されました。終了します。\n");
					break;
				}

				if (key == 'V'){
					sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
				}
				else{
					sprintf_s(strR, "%s\\処理画像\\Effected_%04d.bmp", FolderName, i);
				}
				if (use_src_img) cvReleaseImage(&src_img);
				src_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
				//読み込み失敗
				if (src_img == NULL) {
					printf("撮影画像の読込みに失敗しました.\n");
					continue;
				}
				use_src_img = TRUE;
				cvShowImage("Camera", src_img);
				int delay = 2;
				if (i > 1){ delay = (int)((Ftime[i] - Ftime[i - 1])*1000.0); };
				printf("%s,%s\n", strR, sprintClock(i));
				cvWaitKey(delay);
			}
			if (use_src_img) cvReleaseImage(&src_img);

			cvShowImage("Camera", image1);
			D(key);
		}
		//実験補助動作ここまで-----------------------------------------------------------------------------





		//実験応用動作ここから-----------------------------------------------------------------------------
		if (key == '@'){
			char app[10];
			printf("edge,rgb,color,rgbcolor,mono,op,backのいずれかを入力してください\n実行しない場合は'N'\n");
			scanf_s("%s", app, 10);
			const std::string stra(app);

			char strRT[_MAX_PATH] = "";
			char strRcsv[_MAX_PATH] = "";
			char strRout[_MAX_PATH] = "";
			char strReff[_MAX_PATH] = "";
			int myFILECOUNT;
			int flagM = 0;

			if (!strcmp(app, "N")){
				flagM = 0;
				D(key);
				continue;
			}
			//エッジ検出-----------------------------------------
			else if (!strcmp(app, "edge")){
				IplImage *imgT_in, *imgT_out, *img_in, *img_out;
				_mkdir("実験フォルダ\\エッジ撮影画像");
				_mkdir("実験フォルダ\\エッジ処理画像");

				sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);
				imgT_in = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

				if (imgT_in == NULL){
					printf("not file\n");
					continue;
				}

				imgT_out = cvCreateImage(cvSize(imgT_in->width, imgT_in->height), IPL_DEPTH_8U, 1);
				//エッジ検出の閾値設定
				int n1 = 50, n2 = 150;		//TODO
				cvCanny(imgT_in, imgT_out, n1, n2);

				sprintf_s(strS, "%s\\テンプレートエッジ.bmp", FolderName);
				cvSaveImage(strS, imgT_out);

				printf("テンプレートエッジ.bmp 保存\n");
				printf("%s\n", strS);

				cvReleaseImage(&imgT_in);
				cvReleaseImage(&imgT_out);

				myFILECOUNT = fscanClock();
				if (myFILECOUNT == -1){
					fprintf(stderr, "撮影データがありません.\n");
					fprintf(stderr, "撮影を行ってください.\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}
				for (i = 0; i < myFILECOUNT; i++){
					sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
					img_in = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

					if (img_in == NULL){
						printf("not file %s\n", strR);
						continue;
					}
					img_out = cvCreateImage(cvSize(img_in->width, img_in->height), IPL_DEPTH_8U, 1);
					cvCanny(img_in, img_out, n1, n2);

					sprintf_s(strS, "%s\\エッジ撮影画像\\outputpic_%04d.bmp", FolderName, i);
					cvSaveImage(strS, img_out);

					printf("outputpic_%04d.bmp 保存\n", i);

					cvReleaseImage(&img_in);
					cvReleaseImage(&img_out);
				}
				printf("%s\nエッジ加工が終わりました\n", strS);

				sprintf_s(strRcsv, "%s\\数値データ\\エッジ結果データ.csv", FolderName);
				sprintf_s(strRT, "%s\\テンプレートエッジ.bmp", FolderName);
				sprintf_s(strRout, "%s\\エッジ撮影画像\\outputpic_", FolderName);
				sprintf_s(strReff, "%s\\エッジ処理画像\\Effected_", FolderName);
				flagM = 1;
			}

			//RGB分離後にエッジ検出---------------------------------------
			else if (!strcmp(app, "rgb")){
				IplImage *imgT_in, *imgT_out, *imgET_out, *img_in, *img_out, *imgE_out;
				_mkdir("実験フォルダ\\RGB撮影画像");
				_mkdir("実験フォルダ\\RGBエッジ撮影画像");
				_mkdir("実験フォルダ\\RGBエッジ処理画像");

				int e_temp;
				CvScalar ave;
				IplImage *img_r, *img_g, *img_b, *img_R, *img_G, *img_B;

				myFILECOUNT = fscanClock();
				if (myFILECOUNT == -1){
					fprintf(stderr, "撮影データがありません.\n");
					fprintf(stderr, "撮影を行ってください.\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}

				sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);
				imgT_in = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

				if (imgT_in == NULL){
					printf("not file\n");
					continue;
				}

				img_R = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 1);
				img_G = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 1);
				img_B = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 1);

				img_r = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 3);
				img_g = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 3);
				img_b = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 3);

				cvSetZero(img_r);
				cvSetZero(img_g);
				cvSetZero(img_b);

				ave = cvAvg(imgT_in);
				/*printf("R = %3d\n", ave.val[2]);
				printf("G = %3d\n", ave.val[1]);
				printf("B = %3d\n", ave.val[0]);*/

				cvSplit(imgT_in, img_R, img_G, img_B, NULL);

				cvMerge(NULL, NULL, img_R, NULL, img_r);
				cvMerge(NULL, img_G, NULL, NULL, img_g);
				cvMerge(img_B, NULL, NULL, NULL, img_b);

				if ((ave.val[2] > ave.val[1]) && (ave.val[2] > ave.val[0]))e_temp = 2;
				else if ((ave.val[1] > ave.val[2]) && (ave.val[1] > ave.val[0]))e_temp = 1;
				else e_temp = 0;

				imgT_out = cvCreateImage(cvSize(imgT_in->width, imgT_in->height), IPL_DEPTH_8U, 1);
				if (e_temp == 2)imgT_out = img_r;
				else if (e_temp == 1)imgT_out = img_g;
				else imgT_out = img_b;

				sprintf_s(strS, "%s\\テンプレートRGB.bmp", FolderName);
				cvSaveImage(strS, imgT_out);

				imgET_out = cvCreateImage(cvSize(imgT_in->width, imgT_in->height), IPL_DEPTH_8U, 1);

				cvCanny(imgT_out, imgET_out, 50, 150);

				sprintf_s(strS, "%s\\テンプレートRGBE.bmp", FolderName);
				cvSaveImage(strS, imgET_out);

				printf("テンプレートRGBE.bmp 保存\n");
				printf("%s\n", strS);

				cvReleaseImage(&imgT_in);
				cvReleaseImage(&imgT_out);
				cvReleaseImage(&imgET_out);

				for (i = 0; i < myFILECOUNT; i++){
					sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
					img_in = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

					if (img_in == NULL){
						printf("not file\n");
						continue;
					}

					img_R = cvCreateImage(cvGetSize(img_in), IPL_DEPTH_8U, 1);
					img_G = cvCreateImage(cvGetSize(img_in), IPL_DEPTH_8U, 1);
					img_B = cvCreateImage(cvGetSize(img_in), IPL_DEPTH_8U, 1);

					img_r = cvCreateImage(cvGetSize(img_in), IPL_DEPTH_8U, 3);
					img_g = cvCreateImage(cvGetSize(img_in), IPL_DEPTH_8U, 3);
					img_b = cvCreateImage(cvGetSize(img_in), IPL_DEPTH_8U, 3);

					cvSetZero(img_r);
					cvSetZero(img_g);
					cvSetZero(img_b);

					cvSplit(img_in, img_R, img_G, img_B, NULL);

					cvMerge(NULL, NULL, img_R, NULL, img_r);
					cvMerge(NULL, img_G, NULL, NULL, img_g);
					cvMerge(img_B, NULL, NULL, NULL, img_b);

					img_out = cvCreateImage(cvSize(img_in->width, img_in->height), IPL_DEPTH_8U, 1);
					if (e_temp == 2)img_out = img_r;
					else if (e_temp == 1)img_out = img_g;
					else img_out = img_b;

					sprintf_s(strS, "%s\\RGB撮影画像\\outputpic_%04d.bmp", FolderName, i);
					cvSaveImage(strS, img_out);

					imgE_out = cvCreateImage(cvSize(img_out->width, img_out->height), IPL_DEPTH_8U, 1);

					cvCanny(img_out, imgE_out, 50, 150);

					sprintf_s(strS, "%s\\RGBエッジ撮影画像\\outputpic_%04d.bmp", FolderName, i);
					cvSaveImage(strS, imgE_out);

					printf("outputpic_%04d.bmp 保存\n", i);

					cvReleaseImage(&img_in);
					cvReleaseImage(&img_out);
					cvReleaseImage(&imgE_out);
				}
				printf("%s\nRGB分離後のエッジ加工が終わりました\n", strS);

				sprintf_s(strRcsv, "%s\\数値データ\\RGB結果データ.csv", FolderName);
				sprintf_s(strRT, "%s\\テンプレートRGBE.bmp", FolderName);
				sprintf_s(strRout, "%s\\RGBエッジ撮影画像\\outputpic_", FolderName);
				sprintf_s(strReff, "%s\\RGBエッジ処理画像\\Effected_", FolderName);
				flagM = 1;
			}
			//色検出-------------------------------------------
			else if (!strcmp(app, "color")){
				_mkdir("実験フォルダ\\色検出撮影画像");
				_mkdir("実験フォルダ\\色検出処理画像");
				myFILECOUNT = fscanClock();
				if (myFILECOUNT == -1){
					fprintf(stderr, "撮影データがありません.\n");
					fprintf(stderr, "撮影を行ってください.\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}

				sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);//画像選択
				cv::Mat input_imgT = cv::imread(strR, 1);
				if (input_imgT.empty()){
					printf("not file\n");
					continue;
				}
				cv::Mat smooth_imgT;
				cv::Mat hsv_imgT;
				cv::medianBlur(input_imgT, smooth_imgT, 7);	//ノイズがあるので平滑化
				cv::cvtColor(smooth_imgT, hsv_imgT, CV_BGR2HSV);	//HSVに変換
				int h_ave = 0, s_ave = 0, v_ave = 0;
				int winT = input_imgT.rows, heiT = input_imgT.cols, winhei;
				double rs = 0.0, rv = 0.0, th = 0.0, xT = 0.0, yT = 0.0;
				for (int y = 0; y < winT; y++)
				{
					for (int x = 0; x < heiT; x++)
					{
						int a1 = hsv_imgT.step*y + (x * 3);
						s_ave += hsv_imgT.data[a1 + 1];
						v_ave += hsv_imgT.data[a1 + 2];
						//printf("%d,", hsv_imgT.data[a1]);		//テンプレート画像の色相表示
						th = hsv_imgT.data[a1] * M_PI / 180.0;

						rs = hsv_imgT.data[a1 + 1] / 255.0;
						rv = hsv_imgT.data[a1 + 2] / 255.0;
						xT += rs * rv * cos(2 * th);
						yT += rs * rv * sin(2 * th);
					}
					//printf("\n");
				}
				winhei = winT * heiT;

				//printf("x,y = %lf,%lf\n", xT, yT);
				th = atan2(yT, xT) / 2;
				//printf("%lf\n", th);

				h_ave = (int)(th * 180.0 / M_PI);
				if (h_ave < 0){
					h_ave += 180;
				}

				s_ave = s_ave / winhei;
				v_ave = v_ave / winhei;

				printf("H = %3d\n", h_ave);
				printf("S = %3d\n", s_ave);
				printf("V = %3d\n", v_ave);

				int h_ave1 = 0, s_ave1 = 0, v_ave1 = 0;
				int h_ave2 = 0, s_ave2 = 0, v_ave2 = 0;
				int flag = 0;
				//表示範囲
				int hn = 20;					//TODO
				int sn1 = 100, sn2 = 150;		//TODO
				int vn1 = 100, vn2 = 150;		//TODO

				h_ave1 = imgaveh1(h_ave, hn); h_ave2 = imgaveh2(h_ave, hn);
				s_ave1 = imgavemin(s_ave, sn1); s_ave2 = imgavemax(s_ave, sn2);
				v_ave1 = imgavemin(v_ave, vn1); v_ave2 = imgavemax(v_ave, vn2);

				printf("H12 = (%3d,%3d)\n", h_ave1, h_ave2);
				printf("S12 = (%3d,%3d)\n", s_ave1, s_ave2);
				printf("V12 = (%3d,%3d)\n", v_ave1, v_ave2);
				if (h_ave1 > h_ave2){
					flag = 1;
					printf("flag = %d\n", flag);
				}

				cv::Mat hsv_skin_imgT = cv::Mat(cv::Size(heiT, winT), CV_8UC3);
				hsv_skin_imgT = cv::Scalar(0, 0, 0);

				for (int y = 0; y < winT; y++)
				{
					for (int x = 0; x < heiT; x++)
					{
						int a = hsv_imgT.step*y + (x * 3);
						if (flag == 0 && (hsv_imgT.data[a + 1] > s_ave1 && hsv_imgT.data[a + 1] < s_ave2) && (hsv_imgT.data[a + 2] > v_ave1 && hsv_imgT.data[a + 2] < v_ave2)){
							if (hsv_imgT.data[a] > h_ave1 && hsv_imgT.data[a] < h_ave2) //HSVでの検出
							{
								hsv_skin_imgT.data[a] = 255;
								hsv_skin_imgT.data[a + 1] = 255;
								hsv_skin_imgT.data[a + 2] = 255;
							}
						}
						else if (flag == 1 && (hsv_imgT.data[a + 1] > s_ave1 && hsv_imgT.data[a + 1] < s_ave2) && (hsv_imgT.data[a + 2] > v_ave1 && hsv_imgT.data[a + 2] < v_ave2)){
							if ((hsv_imgT.data[a] > 0 && hsv_imgT.data[a] <= h_ave2) || (hsv_imgT.data[a] >= h_ave1 && hsv_imgT.data[a] < 180)) //HSVでの検出
							{
								hsv_skin_imgT.data[a] = 255;
								hsv_skin_imgT.data[a + 1] = 255;
								hsv_skin_imgT.data[a + 2] = 255;
							}
						}
					}
				}

				sprintf_s(strS, "%s\\テンプレート色検出.bmp", FolderName);
				cv::imwrite(strS, hsv_skin_imgT);
				printf("テンプレート色検出.bmp 保存\n");

				for (i = 0; i < myFILECOUNT; i++){
					sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
					cv::Mat input_img = cv::imread(strR, 1);
					if (input_img.empty()){
						printf("not file\n");
						continue;
					}
					int win = input_img.rows, hei = input_img.cols;
					cv::Mat hsv_skin_img = cv::Mat(cv::Size(hei, win), CV_8UC3);
					cv::Mat smooth_img;
					cv::Mat hsv_img;

					hsv_skin_img = cv::Scalar(0, 0, 0);
					cv::medianBlur(input_img, smooth_img, 7);	//ノイズがあるので平滑化
					cv::cvtColor(smooth_img, hsv_img, CV_BGR2HSV);	//HSVに変換

					for (int y = 0; y < win; y++)
					{
						for (int x = 0; x < hei; x++)
						{
							int a = hsv_img.step*y + (x * 3);
							//if (flag == 0){
							//if (flag == 0 && (hsv_img.data[a + 1] > s_ave1 && hsv_img.data[a + 1] < s_ave2)){
							if (flag == 0 && (hsv_img.data[a + 1] > s_ave1 && hsv_img.data[a + 1] < s_ave2) && (hsv_img.data[a + 2] > v_ave1 && hsv_img.data[a + 2] < v_ave2)){
								if (hsv_img.data[a] > h_ave1 && hsv_img.data[a] < h_ave2) //HSVでの検出
								{
									hsv_skin_img.data[a] = 255;
									hsv_skin_img.data[a + 1] = 255;
									hsv_skin_img.data[a + 2] = 255;
								}
							}
							//else if (flag == 1){
							//else if (flag == 1 && (hsv_img.data[a + 1] > s_ave1 && hsv_img.data[a + 1] < s_ave2)){
							else if (flag == 1 && (hsv_img.data[a + 1] > s_ave1 && hsv_img.data[a + 1] < s_ave2) && (hsv_img.data[a + 2] > v_ave1 && hsv_img.data[a + 2] < v_ave2)){
								if ((hsv_img.data[a] > 0 && hsv_img.data[a] <= h_ave2) || (hsv_img.data[a] >= h_ave1 && hsv_img.data[a] < 180)) //HSVでの検出
								{
									hsv_skin_img.data[a] = 255;
									hsv_skin_img.data[a + 1] = 255;
									hsv_skin_img.data[a + 2] = 255;
								}
							}
						}
					}
					sprintf_s(strS, "%s\\色検出撮影画像\\outputpic_%04d.bmp", FolderName, i);
					cv::imwrite(strS, hsv_skin_img);
					printf("outputpic_%04d.bmp 保存\n", i);
				}
				printf("色検出が終わりました\n");

				sprintf_s(strRcsv, "%s\\数値データ\\色検出結果データ.csv", FolderName);
				sprintf_s(strRT, "%s\\テンプレート色検出.bmp", FolderName);
				sprintf_s(strRout, "%s\\色検出撮影画像\\outputpic_", FolderName);
				sprintf_s(strReff, "%s\\色検出処理画像\\Effected_", FolderName);
				flagM = 1;
			}
			//RGB色検出-------------------------------------------
			else if (!strcmp(app, "rgbcolor")){
				_mkdir("実験フォルダ\\RGB色検出撮影画像");
				_mkdir("実験フォルダ\\RGB色検出処理画像");
				myFILECOUNT = fscanClock();
				if (myFILECOUNT == -1){
					fprintf(stderr, "撮影データがありません.\n");
					fprintf(stderr, "撮影を行ってください.\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}

				IplImage *imgT_in;
				int  r_ave, g_ave, b_ave, e_temp;
				CvScalar ave;
				IplImage *img_r, *img_g, *img_b, *img_R, *img_G, *img_B;

				//テンプレートからRGBを抽出---------------------------------------
				sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);//画像選択
				imgT_in = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
				if (imgT_in == NULL){
					printf("not file\n");
					continue;
				}

				img_R = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 1);
				img_G = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 1);
				img_B = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 1);

				img_r = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 3);
				img_g = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 3);
				img_b = cvCreateImage(cvGetSize(imgT_in), IPL_DEPTH_8U, 3);

				cvSetZero(img_r); cvSetZero(img_g); cvSetZero(img_b);

				cvSplit(imgT_in, img_R, img_G, img_B, NULL);

				cvMerge(NULL, NULL, img_R, NULL, img_r);
				cvMerge(NULL, img_G, NULL, NULL, img_g);
				cvMerge(img_B, NULL, NULL, NULL, img_b);

				ave = cvAvg(imgT_in);
				r_ave = (int)ave.val[2];
				g_ave = (int)ave.val[1];
				b_ave = (int)ave.val[0];

				if ((ave.val[2] > ave.val[1]) && (ave.val[2] > ave.val[0])){
					e_temp = 2;	printf("red\n");
				}
				else if ((ave.val[1] > ave.val[2]) && (ave.val[1] > ave.val[0])){
					e_temp = 1; printf("green\n");
				}
				else {
					e_temp = 0; printf("brue\n");
				}

				//------------------------------------------------------------

				cv::Mat input_imgT = cv::imread(strR, 1);
				if (input_imgT.empty()){
					printf("not file\n");
					continue;
				}
				cv::Mat smooth_imgT;
				cv::Mat hsv_imgT;
				cv::medianBlur(input_imgT, smooth_imgT, 7);	//ノイズがあるので平滑化
				cv::cvtColor(smooth_imgT, hsv_imgT, CV_BGR2HSV);	//HSVに変換
				int s_ave = 0, v_ave = 0;
				int winT = input_imgT.rows, heiT = input_imgT.cols, winhei;
				double r = 0.0, th = 0.0, xT = 0.0, yT = 0.0;
				for (int y = 0; y < winT; y++)
				{
					for (int x = 0; x < heiT; x++)
					{
						int a1 = hsv_imgT.step*y + (x * 3);
						s_ave += hsv_imgT.data[a1 + 1];
						v_ave += hsv_imgT.data[a1 + 2];
					}
				}
				winhei = winT * heiT;
				s_ave = s_ave / winhei;
				v_ave = v_ave / winhei;

				int s_ave1 = 0, v_ave1 = 0;
				int s_ave2 = 0, v_ave2 = 0;
				int sn1 = 50, sn2 = 150, vn1 = 50, vn2 = 150;		//表示範囲

				s_ave1 = imgavemin(s_ave, sn1); s_ave2 = imgavemax(s_ave, sn2);
				v_ave1 = imgavemin(v_ave, vn1); v_ave2 = imgavemax(v_ave, vn2);

				cv::Mat hsv_skin_imgT = cv::Mat(cv::Size(heiT, winT), CV_8UC3);
				hsv_skin_imgT = cv::Scalar(0, 0, 0);

				for (int y = 0; y < winT; y++)
				{
					for (int x = 0; x < heiT; x++)
					{
						int a = hsv_imgT.step*y + (x * 3);
						if (e_temp == 2 && (hsv_imgT.data[a + 1] > s_ave1 && hsv_imgT.data[a + 1] < s_ave2) && (hsv_imgT.data[a + 2] > v_ave1 && hsv_imgT.data[a + 2] < v_ave2)){
							if ((hsv_imgT.data[a] > 0 && hsv_imgT.data[a] <= 30) || (hsv_imgT.data[a] >= 150 && hsv_imgT.data[a] < 180))
							{
								hsv_skin_imgT.data[a] = 0;
								hsv_skin_imgT.data[a + 1] = 0;
								hsv_skin_imgT.data[a + 2] = 255;
							}
						}
						else if (e_temp == 1 && (hsv_imgT.data[a + 1] > s_ave1 && hsv_imgT.data[a + 1] < s_ave2) && (hsv_imgT.data[a + 2] > v_ave1 && hsv_imgT.data[a + 2] < v_ave2)){
							if ((hsv_imgT.data[a] >= 30 && hsv_imgT.data[a] <= 90))
							{
								hsv_skin_imgT.data[a] = 0;
								hsv_skin_imgT.data[a + 1] = 255;
								hsv_skin_imgT.data[a + 2] = 0;
							}
						}
						else if (e_temp == 0 && (hsv_imgT.data[a + 1] > s_ave1 && hsv_imgT.data[a + 1] < s_ave2) && (hsv_imgT.data[a + 2] > v_ave1 && hsv_imgT.data[a + 2] < v_ave2)){
							if ((hsv_imgT.data[a] >= 90 && hsv_imgT.data[a] <= 150))
							{
								hsv_skin_imgT.data[a] = 255;
								hsv_skin_imgT.data[a + 1] = 0;
								hsv_skin_imgT.data[a + 2] = 0;
							}
						}
					}
				}

				sprintf_s(strS, "%s\\テンプレートrgb色検出.bmp", FolderName);
				cv::imwrite(strS, hsv_skin_imgT);

				for (int i = 0; i < myFILECOUNT; i++){
					sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
					cv::Mat input_img = cv::imread(strR, 1);
					if (input_img.empty()){
						printf("not file\n");
						continue;
					}
					int win = input_img.rows, hei = input_img.cols;
					cv::Mat hsv_skin_img = cv::Mat(cv::Size(hei, win), CV_8UC3);
					cv::Mat smooth_img;
					cv::Mat hsv_img;

					hsv_skin_img = cv::Scalar(0, 0, 0);
					cv::medianBlur(input_img, smooth_img, 7);	//ノイズがあるので平滑化
					cv::cvtColor(smooth_img, hsv_img, CV_BGR2HSV);	//HSVに変換

					printf("%d\n", i);
					for (int y = 0; y < win; y++)
					{
						for (int x = 0; x < hei; x++)
						{
							int a = hsv_img.step*y + (x * 3);
							if (e_temp == 2 && (hsv_img.data[a + 1] > s_ave1 && hsv_img.data[a + 1] < s_ave2) && (hsv_img.data[a + 2] > v_ave1 && hsv_img.data[a + 2] < v_ave2)){
								if ((hsv_img.data[a] > 0 && hsv_img.data[a] <= 30) || (hsv_img.data[a] >= 150 && hsv_img.data[a] < 180))
								{
									hsv_skin_img.data[a] = 0;
									hsv_skin_img.data[a + 1] = 0;
									hsv_skin_img.data[a + 2] = 255;
								}
							}
							else if (e_temp == 1 && (hsv_img.data[a + 1] > s_ave1 && hsv_img.data[a + 1] < s_ave2) && (hsv_img.data[a + 2] > v_ave1 && hsv_img.data[a + 2] < v_ave2)){
								if (hsv_img.data[a] >= 30 && hsv_img.data[a] <= 90)
								{
									hsv_skin_img.data[a] = 0;
									hsv_skin_img.data[a + 1] = 255;
									hsv_skin_img.data[a + 2] = 0;
								}
							}
							else if (e_temp == 0 && (hsv_img.data[a + 1] > s_ave1 && hsv_img.data[a + 1] < s_ave2) && (hsv_img.data[a + 2] > v_ave1 && hsv_img.data[a + 2] < v_ave2)){
								if (hsv_img.data[a] >= 90 && hsv_img.data[a] <= 150)
								{
									hsv_skin_img.data[a] = 255;
									hsv_skin_img.data[a + 1] = 0;
									hsv_skin_img.data[a + 2] = 0;
								}
							}
						}
					}

					sprintf_s(strS, "%s\\RGB色検出撮影画像\\outputpic_%04d.bmp", FolderName, i);
					cv::imwrite(strS, hsv_skin_img);
				}

				printf("RGB色検出が終わりました\n");

				sprintf_s(strRcsv, "%s\\数値データ\\RGB色検出結果データ.csv", FolderName);
				sprintf_s(strRT, "%s\\テンプレートRGB色検出.bmp", FolderName);
				sprintf_s(strRout, "%s\\RGB色検出撮影画像\\outputpic_", FolderName);
				sprintf_s(strReff, "%s\\RGB色検出処理画像\\Effected_", FolderName);
				flagM = 1;
			}
			//物検出の背景色設定-------------------------------------------
			else if (!strcmp(app, "back")){
				printf("Rの値(0～255),Gの値(0～255),Bの値(0～255)を空白で区切って3つ入力してください。\n");

				printf("現在の（R,G,B）の数値 :（ %d, %d, %d )\n", back_R, back_G, back_B);
				int appR, appG, appB;
				std::cin >> appR >> appG >> appB;    // キーボードから入力を受ける
				if (appR >= 0 && appR <= 255){
					back_R = appR;
				}
				if (appG >= 0 && appG <= 255){
					back_G = appG;
				}
				if (appB >= 0 && appB <= 255){
					back_B = appB;
				}

				printf("変更後の（R,G,B）の数値 :（ %d, %d, %d )\n", back_R, back_G, back_B);

				flagM = 0;
				cin.clear();
				cin.ignore();
				D(key);
			}
			//物検出-------------------------------------------
			else if (!strcmp(app, "mono")){
				_mkdir("実験フォルダ\\撮影差分画像");
				_mkdir("実験フォルダ\\処理差分画像");
				myFILECOUNT = fscanClock();
				if (myFILECOUNT == -1){
					fprintf(stderr, "撮影データがありません.\n");
					fprintf(stderr, "撮影を行ってください.\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}

				IplImage *frame, *frame_h, *img_diff, *img_diff_1;	// 画像リソース宣言
				CvMat *mat_src, *mat_src_h, *mat_diff, *mat_diff1;	// 行列リソース宣言
				IplImage *img_out;
				CvSeq *circles = 0;
				cv::Mat mat_out2;
				cv::Mat mat_out;
				cv::Mat frame_in;
				int win, hei;
				char strR[_MAX_PATH] = "";
				char strS[_MAX_PATH] = "";
				sprintf_s(strR, "%s\\背景.bmp", FolderName);
				frame_h = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
				if (frame_h == NULL){
					printf("not file\n'b'を押して背景画像を撮影してください\n");
					continue;
				}
				frame = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
				if (frame == NULL){
					printf("not file\n");
					continue;
				}

				// 画像リソース確保
				img_diff = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
				img_diff_1 = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);
				img_out = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);
				// 32bit浮動小数点数型3チャネルの行列リソースを準備
				mat_diff = cvCreateMat(frame->height, frame->width, CV_32FC3);
				mat_diff1 = cvCreateMat(frame->height, frame->width, CV_32FC3);
				mat_src = cvCreateMat(frame->height, frame->width, CV_32FC3);
				mat_src_h = cvCreateMat(frame->height, frame->width, CV_32FC3);

				cvSetZero(mat_diff); cvSetZero(mat_src); // 0で初期化
				cvSetZero(mat_diff1); cvSetZero(mat_src_h);
				cvConvert(frame_h, mat_src_h); // 入力画像を浮動小数点数型行列に変換

				for (i = 0; i < myFILECOUNT; i++){

					printf("%d\n", i);
					sprintf_s(strR, "%s\\撮影画像\\outputpic_%04d.bmp", FolderName, i);
					frame = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
					if (frame == NULL){
						printf("not file\n");
						continue;
					}
					cvConvert(frame, mat_src); // 入力画像を浮動小数点数型行列に変換

					cvAbsDiff(mat_src_h, mat_src, mat_diff1);	// 差分の計算

					cvConvert(mat_diff1, img_diff);	// 浮動小数点数型行列を画像に変換

					//閾値処理(cvThreshold)での閾値
					int ts1 = 10, ts2 = 250;	//TODO
					cvCvtColor(img_diff, img_diff_1, CV_BGR2GRAY);
					cvThreshold(img_diff_1, img_out, ts1, ts2, CV_THRESH_BINARY);

					sprintf_s(strS, "%s\\撮影差分画像\\outputpic_%04d.bmp", FolderName, i);
					cvSaveImage(strS, img_out);

					frame_in = cv::imread(strR);
					if (frame_in.empty()){
						printf("not file\n");
						continue;
					}
					mat_out = cv::imread(strS);
					if (mat_out.empty()){
						printf("not file\n");
						continue;
					}
					win = mat_out.rows;
					hei = mat_out.cols;
					mat_out2 = cv::Mat(cv::Size(hei, win), CV_8UC3);

					for (int y = 0; y < win; y++)
					{
						for (int x = 0; x < hei; x++)
						{
							int a1 = mat_out.step*y + (x * 3);
							if (!(mat_out.data[a1] == 0)){//背景画像との差分があるとき
								mat_out2.data[a1] = frame_in.data[a1];
								mat_out2.data[a1 + 1] = frame_in.data[a1 + 1];
								mat_out2.data[a1 + 2] = frame_in.data[a1 + 2];
							}
							else{
								mat_out2.data[a1] = back_B;
								mat_out2.data[a1 + 1] = back_G;
								mat_out2.data[a1 + 2] = back_R;
							}
						}
					}
					cv::imwrite(strS, mat_out2);
				}
				// ウィンドウ・キャプチャ・画像リソースの解放
				cvReleaseImage(&img_diff); cvReleaseImage(&img_diff_1); cvReleaseImage(&img_out);
				cvReleaseMat(&mat_src);	cvReleaseMat(&mat_diff); cvReleaseMat(&mat_diff1); cvReleaseMat(&mat_src_h);

				printf("物体検出が終わりました\n");

				sprintf_s(strRcsv, "%s\\数値データ\\差分結果データ.csv", FolderName);
				sprintf_s(strRT, "%s\\テンプレート.bmp", FolderName);
				sprintf_s(strRout, "%s\\撮影差分画像\\outputpic_", FolderName);
				sprintf_s(strReff, "%s\\処理差分画像\\Effected_", FolderName);
				flagM = 1;
			}
			else if (!strcmp(app, "op")){
				printf("\nオプティカルフロー、モーション履歴を出力します\nSpace keyで終了します\n");
				//オプティカルフロー
				int i, count = COUNT;
				char status[COUNT];
				CvPoint2D32f feature_pre[COUNT];	// 浮動小数点数型座標の特徴点
				CvPoint2D32f feature_now[COUNT];
				CvCapture* src_op;								// ビデオキャプチャ宣言
				IplImage *frame_in, *frame_now, *frame_pre, *img_out;	// 画像リソース宣言
				IplImage *img_tmp1, *img_tmp2, *pyramid_now, *pyramid_pre;
				double th, h, S = 255, V = 255, r, g, b, max, min;

				// 反復アルゴリズム用終了条件
				CvTermCriteria criteria;
				criteria = cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);

				src_op = cvCaptureFromCAM(defaultCAM); // 映像取得（カメラ映像）
				if (src_op == NULL){ printf("映像が取得できません。\n"); continue; }

				frame_in = cvQueryFrame(src_op); // 初期フレーム取得
				frame_now = cvCreateImage(cvGetSize(frame_in), IPL_DEPTH_8U, 1); // 画像リソース確保
				frame_pre = cvCreateImage(cvGetSize(frame_in), IPL_DEPTH_8U, 1);
				img_out = cvCreateImage(cvGetSize(frame_in), IPL_DEPTH_8U, 3);
				img_tmp1 = cvCreateImage(cvGetSize(frame_in), IPL_DEPTH_32F, 1);
				img_tmp2 = cvCreateImage(cvGetSize(frame_in), IPL_DEPTH_32F, 1);
				pyramid_pre = cvCreateImage(cvSize(frame_in->width + 8, frame_in->height / 3),
					IPL_DEPTH_8U, 1);
				pyramid_now = cvCreateImage(cvSize(frame_in->width + 8, frame_in->height / 3),
					IPL_DEPTH_8U, 1);
				frame_in = cvQueryFrame(src_op);					// 現在フレーム取得
				cvCvtColor(frame_in, frame_pre, CV_BGR2GRAY);	// グレースケール変換（事前画像準備）

				//モーション履歴
				double time_s, angle;
				CvSize size;
				IplImage *frame, *frame_now_m, *frame_pre_m, *diff;	// 画像変数宣言
				IplImage *hist, *hist_8, *hist_32, *direction;

				frame = cvQueryFrame(src_op);	// 初期フレーム取得
				size = cvSize(frame->width, frame->height);	// 入力サイズ取得

				// 画像領域確保・初期化
				frame_pre_m = cvCreateImage(size, IPL_DEPTH_8U, 1); cvZero(frame_pre_m);
				frame_now_m = cvCreateImage(size, IPL_DEPTH_8U, 1); cvZero(frame_now_m);
				diff = cvCreateImage(size, IPL_DEPTH_8U, 1);	// 差分画像
				hist = cvCreateImage(size, IPL_DEPTH_8U, 3);	// 履歴画像（整数型3チャネル）
				hist_8 = cvCreateImage(size, IPL_DEPTH_8U, 1);	// 履歴画像（整数型1チャネル）
				hist_32 = cvCreateImage(size, IPL_DEPTH_32F, 1);	// 履歴画像（浮動小数点数型1チャネル）
				direction = cvCreateImage(size, IPL_DEPTH_32F, 1);	// 方向画像

				cvNamedWindow("入力映像");
				cvNamedWindow("オプティカルフロー");
				cvNamedWindow("モーションテンプレート");

				while (1){
					//オプティカルフロー
					frame_in = cvQueryFrame(src_op); if (frame_in == NULL) break;	// 現在フレーム取得

					cvCvtColor(frame_in, frame_now, CV_BGR2GRAY);				// グレースケール変換

					cvGoodFeaturesToTrack(frame_pre, img_tmp1, img_tmp2,		// 特徴点の抽出
						feature_pre, &count, 0.001, 5, NULL);
					// オプティカルフロー検出
					cvCalcOpticalFlowPyrLK(frame_pre, frame_now, pyramid_pre, pyramid_now,
						feature_pre, feature_now, count, cvSize(10, 10),
						4, status, NULL, criteria, 0);

					float line_x = 0.0, line_y = 0.0;
					cvSetZero(img_out);				// 結果画像初期化
					for (i = 0; i < count; i++){		// オプティカルフロー描画
						line_x = feature_pre[i].x - feature_now[i].x;
						line_y = feature_pre[i].y - feature_now[i].y;

						th = atan2(line_y, line_x) / 2;
						h = (th * 180.0 / M_PI);
						max = V; min = max - ((S / 255) * max);
						if (h < 0){
							h += 180;
						}
						h = h * 2;

						if (h > 0 && h <= 60){
							r = max;
							g = (h / 60.0) * (max - min) + min;
							b = min;
						}
						else if (h > 60 && h <= 120){
							r = ((120.0 - h) / 60.0) * (max - min) + min;
							g = max;
							b = min;
						}
						else if (h > 120 && h <= 180){
							r = min;
							g = max;
							b = ((h - 120.0) / 60.0) * (max - min) + min;
						}
						else if (h > 180 && h <= 240){
							r = min;
							g = ((240.0 - h) / 60.0) * (max - min) + min;
							b = max;
						}
						else if (h > 240 && h <= 300){
							r = ((h - 240.0) / 60.0) * (max - min) + min;
							g = min;
							b = max;
						}
						else if (h > 300 && h <= 360){
							r = max;
							g = min;
							b = ((360.0 - h) / 60.0) * (max - min) + min;
						}

						cvLine(img_out, cvPointFrom32f(feature_pre[i]),
							cvPointFrom32f(feature_now[i]), CV_RGB(b, g, r), 1, CV_AA, 0);
					}

					cvFlip(frame_in, frame_in, 1);
					cvFlip(img_out, img_out, 1);

					cvShowImage("入力映像", frame_in);
					cvShowImage("オプティカルフロー", img_out);
					cvCopy(frame_now, frame_pre);	// 現在画像を事前画像にコピー

					//モーション履歴
					frame = cvQueryFrame(src_op); if (frame == NULL) break;		// 1フレーム取得

					cvCvtColor(frame, frame_now_m, CV_BGR2GRAY);				// グレースケール変換
					cvAbsDiff(frame_pre_m, frame_now_m, diff);					// 直前フレームとの差分を抽出
					cvThreshold(diff, diff, 60, 1, CV_THRESH_BINARY);		// 差分を2値化
					time_s = (double)clock() / CLOCKS_PER_SEC;				// 現在時間（秒）の取得
					cvUpdateMotionHistory(diff, hist_32, time_s, 1);		// 履歴画像を作成
					cvConvertScale(hist_32, hist_8, 255, -(time_s - 1) * 255);	// 0～255にスケーリング
					cvZero(hist);											// 履歴画像を初期化
					cvMerge(hist_8, 0, 0, 0, hist);				// 履歴画像を3チャネルカラー画像化
					cvCalcMotionGradient(hist_32, hist_8, direction, 0.5, 0.05, 3);	// 方向画像作成
					// 履歴画像と方向画像から全体の動き方向を計算
					angle = 360 - cvCalcGlobalOrientation(direction, hist_8, hist_32, time_s, 1);

					cvLine(hist, cvPoint(size.width / 2, size.height / 2),		// 動き方向ラインを描画
						cvPoint(size.width / 2 + int(100 * cos(angle*CV_PI / 180)),
						size.height / 2 - int(100 * sin(angle*CV_PI / 180))),
						CV_RGB(255, 0, 0), 3, CV_AA, 0);

					cvFlip(hist, hist, 1);			// 左右反転（鏡面モード）
					cvCopy(frame_now_m, frame_pre_m);	// 現在フレームを次ループの直前フレームとして保持

					cvShowImage("モーションテンプレート", hist);	// 履歴画像1フレーム表示

					if (cvWaitKey(30) == 32)
					{
						printf("\n終了します\n");
						break;	// Space keyを押した時終了
					}
				}
				// ウィンドウ・キャプチャ・画像リソースの解放
				cvDestroyWindow("モーションテンプレート");
				cvDestroyWindow("入力映像");
				cvDestroyWindow("オプティカルフロー");
				cvReleaseCapture(&src_op);
				cvReleaseImage(&frame_now);	cvReleaseImage(&frame_pre);	cvReleaseImage(&img_out);
				cvReleaseImage(&img_tmp1); cvReleaseImage(&img_tmp2); cvReleaseImage(&pyramid_pre);
				cvReleaseImage(&pyramid_now);
				cvReleaseImage(&frame_now_m);	cvReleaseImage(&frame_pre_m);	cvReleaseImage(&diff);
				cvReleaseImage(&hist);		cvReleaseImage(&hist_8);	cvReleaseImage(&hist_32);
				cvReleaseImage(&direction);
				D(key);
				defaultCAM = defaultCAM;
			}
			else{
				flagM = 0;
				D(key);
				continue;
			}

			if (flagM != 0){
				//マッチテンプレート
				flag_Temp = 0;
				int num_bmp = 0;
				int num_effected = 0;
				IplImage *tmp_img;
				double val4files[FILECOUNT_MAX];//類似度
				char XYpoint[FILECOUNT_MAX][20];//座標取得の文字列20文字
				for (int i1 = 0; i1 < FILECOUNT_MAX; i1++){
					val4files[i1] = -0.01;
				}

				printf("\n\n解析処理を行います.\n");
				// precheck for 結果データ
				if (error = fopen_s(&file, strRcsv, "w") != 0){
					printf("%s\n", strRcsv);
					fprintf(stderr, "結果データに書き込めません.処理を中断します.\n");
					fprintf(stderr, "（Excelなどで開いていると書き込めませんので閉じてください.）\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}
				else{ fclose(file); }

				printf("%d個のテンプレートマッチング処理を行います。少々お待ちください。\n", myFILECOUNT);

				//読み込み
				tmp_img = cvLoadImage(strRT, CV_LOAD_IMAGE_COLOR);

				//読み込み失敗
				if (tmp_img == NULL) {
					fprintf(stderr, "テンプレート画像の読込みに失敗しました.\n");
					fprintf(stderr, "テンプレート画像を作成してください.\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}

				//read config.txt
				if (error = fopen_s(&file_config, "./実験フォルダ/Config.txt", "r") != 0){
					fprintf(stderr, "Config.txt の読込みに失敗しました.\n");
					fprintf(stderr, "再起動してください.\n\n");
					cvShowImage("Camera", image1);
					D(key);
					continue;
				}
				else{
					fscanf_s(file_config, "%lf", &config_val);
					fclose(file_config);
				}
				bool use_src_img = FALSE;
				for (i = 0; i < myFILECOUNT; i++){
					if (GetAsyncKeyState(VK_SPACE) & 0x8000){
						printf("Space keyが入力されました。テンプレートマッチング処理を中断します.\n");
						flag_Temp = 1;
						break;
					}
					sprintf_s(strR, "%s%04d.bmp", strRout, i);
					if (use_src_img){
						cvReleaseImage(&src_img);
					}
					src_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
					use_src_img = TRUE;
					//読み込み失敗
					if (src_img == NULL) {
						printf("撮影画像の読込みに失敗しました.\n");
					}
					else{
						num_bmp++;
						printf("outputpic_%04d.bmp:", i);
						// (1)探索画像全体に対して，テンプレートのマッチング値（指定した手法に依存）を計算
						dst_size = cvSize(src_img->width - tmp_img->width + 1, src_img->height - tmp_img->height + 1);
						dst_img = cvCreateImage(dst_size, IPL_DEPTH_32F, 1);
						cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);//methodも変えつつ検証が必要
						cvMinMaxLoc(dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);

						/*150番目の類似度確認用*/
						if (i == 150){
							img_ccoeff = dst_img;
							cvMinMaxLoc(img_ccoeff, &Cmin, &Cmax, &Pmin, &Pmax, NULL);
						}
						cvReleaseImage(&dst_img);
						val4files[i] = max_val;
						if (val4files[i] < config_val){//テンプレートと探索対象の類似度がconfig_val 未満
							//X,Y座標の出力 - for csv
							sprintf_s(XYpoint[i], ",%02d,%02d", 0, 0);
							printf("類似度%.1f(%.1f%%未満)\n", max_val*100.0, config_val*100.0);

							cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(255, 0, 0), 2);//赤枠四角を描画
							cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画

							cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//白縁
							cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font, CV_RGB(200, 0, 0));			//赤文字

							sprintf_s(strR, "%s%04d.bmp", strReff, i);
							cvSaveImage(strR, src_img);
							cvShowImage("Camera", src_img);
							cvWaitKey(1);
						}
						else{
							// (2)テンプレートに対応する位置に矩形を描画、中心点も描画
							cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(0, 0, 255), 2);//青枠四角を描画
							cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画
							//タイムスタンプ

							cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//白縁
							cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font, CV_RGB(0, 0, 0));			//黒文字

							num_effected++;
							sprintf_s(strR, "%s%04d.bmp", strReff, i);
							cvSaveImage(strR, src_img);
							cvShowImage("Camera", src_img);
							cvWaitKey(1);
							test = (max_loc.x + tmp_img->width / 2);
							if (max < test){
								max = test;
								M = i;
							}
							//X,Y座標の出力 - for csv
							sprintf_s(XYpoint[i], ",%02d,%02d", (max_loc.x + tmp_img->width / 2), (max_loc.y + tmp_img->height / 2));
							printf("Effected_%04d.bmp 作成 (類似度%.1f%%)\n", i, max_val*100.0);
						}
					}
				}
				if (flag_Temp == 0){
					if (use_src_img){
						cvReleaseImage(&src_img);
					}
					cvReleaseImage(&tmp_img);
					fprintf(stderr,
						"マッチング処理が終了しました。\n"
						"結果データをCSVファイルに書き込みます。\n"
						);
					//結果データの書き込み
					if (error = fopen_s(&file, strRcsv, "w") != 0){
						printf("%s\n", strT);
						fprintf(stderr, "結果データに書き込めません.\n");
						fprintf(stderr, "（Excelなどで開いていると書き込めません.）\n\n");
						cvShowImage("Camera", image1);
						D(key);
						continue;
					}

					//結果CSV1行目の記述
					fprintf(file, "画像ファイル名,t[s],x[pixel],y[pixel],X[meter],Y[meter],類似度,,←１ピクセルが何メートルか？\n");

					if (num_effected != 0){
						//各行の記述
						int c = 2; // line number of csv file (start from 2)
						int flagcsv = 0;
						double elatime, elatime0;
						char *ale;
						for (int i = 0; i < myFILECOUNT; i++){
							if (val4files[i] < config_val){
								//printf("%d skiped\n",i);
								//getchar();
							}
							//数値データにファイル名など座標、エクセル上での計算式を出力させる
							else{
								if (flagcsv == 0){
									elatime0 = strtod(sprintClock(i), &ale);
									flagcsv = 1;
								}
								char FName[50];
								sprintf_s(FName, "img_%04d.bmp", i);
								fprintf(file, FName);	//ファイル名&時刻データを出力
								elatime = strtod(sprintClock(i), &ale);
								elatime -= elatime0;
								fprintf(file, ",%.6lf", elatime);	//１フレームごとの時間座標を数値データ出力
								fprintf(file, XYpoint[i]);	//XY座標を数値データ出力
								fprintf(file, ",=(C%d-$C$2)*$H$1,=(D%d-$D$2)*$H$1", c, c);	//xとyを数値データ出力
								fprintf(file, ",%.6lf", val4files[i]);
								fprintf(file, "\n");		//改行しないと横1列になる
								c++;
							}
						}
					}
					fclose(file);
					printf("数値データ出力が終了しました.\n");

					printf("\n\n類似度%.1f%%以上の検出結果\n全%d枚中 %d枚検出できました.\n\n", config_val*100.0, num_bmp, num_effected);
					printf("---------検出数が少ない場合の対処---------\n");
					printf("※テンプレート画像やConfig.txtを変更し再度Match Templateの実行.\n");
				}
				cvShowImage("Camera", image1);
				D(key);
				flagM = 0;
			}
			key = 32;
		}
		//実験応用動作ここまで-----------------------------------------------------------------------------
	}
	cvReleaseCapture(&videoCapture1);
	cvDestroyWindow("Camera");
	return 0;
}


/*

この実験ソフトは OpenCV-2.4.9 を使っています。ぜひ開発に参加してください。

OpenCVはBSDラインセンスに基づき、商用/非商用問わず、
無償で使用する事が可能です。以下に、 OpenCV の
ライセンスを表示します。 (opencv-2.4.9\sources\LICENCE より全文)


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