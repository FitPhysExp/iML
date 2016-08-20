// 2015-09-29: maru+shimo version
#include <stdio.h>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/core/core.hpp>
#include <direct.h>
#include <conio.h> // for windows only kbhit and getch


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
		" 0 :このアプリケーションを終了する\n"
		" 7 :マッチングの類似度を変更する\n"
		" 8 :データコピーのためフォルダを開く\n"
		" 9 :処理画像を表示する\n"
		" 実験フォルダがあることを確認して、撮影・解析を行ってください.\n"
		"-------------------------------------------------------------------------\n"
		);
}
void D00(void)		//キー操作一覧のテキスト表示
{
	fprintf(stderr,
		"----バージョン("__DATE__"-"__TIME__")-----------------------------------\n"
		" h or ? : メニューの表示\n"
		" c :カメラ入力の変更\n"
		" t :無制限に画像を保存(Spece Keyの入力で終了)\n"
		" R :テンプレートの練習(Space 終了・ENTER 新モード)\n"
		);
	fprintf(stderr, " d :実験フォルダの作成.\n");
	fprintf(stderr, " b :テンプレート用画像の保存.\n");
	fprintf(stderr, " s :画像を250枚保存\n");

	fprintf(stderr, " m :Match Template　の実行,数値出力.\n");
	fprintf(stderr, " q or Esc : このアプリケーションを終了する\n");
	fprintf(stderr, "-------------------------------------------------------------------------\n\n");
}

//時間出力用
#include <time.h>
#include <windows.h>

/*OpenCV 2.4.9のライブラリ*/
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

#define DEFAULT_CONFIG	0.70 //類似度の初期設定
#define FILECOUNT		250
#define FILECOUNT_MAX   9999
//100枚で3秒程度	(30fps)	//これを小さくし過ぎるとプログラムで使用する配列のサイズが変わる
//バッファサイズが足りず実行できないことがある


/* グローバル変数 */
//	IplImage *img = 0;
const char* FolderName = "実験フォルダ";//フォルダ名
errno_t error;
using namespace std;

void D0(void);
void D(int K);

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

int main(int argc, char **argv)
{

	int i, test, max = 0, M;
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

	int Savecount = -1;//判断処理用変数

	//数値データファイル入出力
	FILE *file, *file_config;

	//フォント - time stamp
	CvFont font, font_w;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 1.0, 0, 1, CV_AA);
	cvInitFont(&font_w, CV_FONT_HERSHEY_SIMPLEX, 0.5, 1.0, 0, 3, CV_AA);

	int defaultCAM = 1;// ThinkPad カメラ用の設定
	CvCapture * videoCapture1 = cvCaptureFromCAM(defaultCAM);
	//１台のみの場合cvCaptureFromCAMの引数はなんでもいい
	//複数台の場合はPC起動時の接続順が引数になる


	//------------------------------
	D0();
	if (_mkdir("実験フォルダ") == 0){													//.exeと同じ階層にディレクトリを作成
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
		fprintf(file_config, "%f", DEFAULT_CONFIG);
		config_val = DEFAULT_CONFIG;
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
		//---------------------------------------------------------------------------------------------------------------
		if (key == 'c'){
			defaultCAM--;
			if (defaultCAM < 0){ defaultCAM = 3; }
			printf("\n\nカメラ番号を%02dへ変更しました.\n画面に動きがあればOKです\n", defaultCAM);
			cvReleaseCapture(&videoCapture1);
			videoCapture1 = cvCaptureFromCAM(defaultCAM);
		}
		//相互相関係数分布
		else if (key == 'C'){
			cvConvertScale(img_ccoeff, img_ccoeff, 1.0 / Cmax, 0.0);
			cvShowImage("相互相関係数分布", img_ccoeff);
		}
		//---------------------------------------------------------------------------------------------------------------
		else if (key == 'f' || key == '2'){
			fprintf(stderr, "テンプレート.bmpを開きますのでお待ちください。\n");
			system("mspaint \".\\実験フォルダ\\テンプレート.bmp\"");												//ペイントで対象を開く
			D(key);
		}
		else if (key == '5'){
			fprintf(stderr, "結果データ.csvを開きますのでお待ちください。\n");
			system("explorer \".\\実験フォルダ\\数値データ\\結果データ.csv\"");												//エクスプローラーで対象を開く
			D(key);
		}
		else if (key == '6'){
			system("xcopy /S /C /I /Y \".\\実験フォルダ\" %date:~-5,2%%date:~-2%%time:~0,2%%time:~3,2%%time:~6,2%");
			D(key);
		}
		else if (key == '7'){
			fprintf(stderr, "Config.txtを開きます。類似度を設定してください。\n");
			system("explorer \".\\実験フォルダ\\Config.txt\"");												//エクスプローラーで対象を開く
			D(key);
		}
		else if (key == '8'){
			system("explorer \".\\実験フォルダ\"");												//エクスプローラーで対象を開く
			D(key);
		}
		else if (key == 'd' || key == 'D'){ //C:\\Users\\yama\\Documents\\実験フォルダ-webCam
			if (_mkdir("実験フォルダ") == 0){													//.exeと同じ階層にディレクトリを作成
				printf("フォルダ作成\n");
			}
			else{
				printf("フォルダ作成に失敗しました。既にフォルダが存在する可能性があります。\n");
			}
			system("explorer \"実験フォルダ");												//エクスプローラーで対象を開く
			_mkdir("実験フォルダ\\数値データ");
			_mkdir("実験フォルダ\\撮影画像");
			_mkdir("実験フォルダ\\処理画像");
			fprintf(stderr, "実験用フォルダ作成を開きます.\n");
			D(key);

		}
		//画像の1枚の保存を行う------------------------------------------------------------------------------
		else if (key == 'b' || key == 'B' || key == '1'){
			IplImage *output = cvQueryFrame(videoCapture1);

			sprintf_s(strB, "%s\\テンプレート.bmp", FolderName);
			cvSaveImage(strB, output);

			fprintf(stderr, "テンプレート用画像の保存に成功しました\n");
			D(key);
		}
		//設定枚数分の画像をメモリ使用し保存する------------------------------------------------------------------------
		else if (key == 's' || key == '3'){
			fprintf(stderr, "画像をメモリに格納します.(Space key で終了).\n");
			cvDestroyAllWindows();
			IplImage *output;
			IplImage** vout;
			vout = (IplImage **)malloc(sizeof(IplImage *)*FILECOUNT);
			int i;
			startClock();
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
				cvWaitKey(2);
				cvSaveImage(strS, vout[i]);
				printf("outputpic_%04d.bmp 保存\n", i);
				cvReleaseImage(&(vout[i]));
			}
			free(vout);

			fprintf(stderr, "撮影画像を保存しました.\n");
			fprintClock();
			//--------------------------------------------------------------------------
			cvShowImage("Camera", image1);
			D(key);
		}
		//設定枚数分の画像をメモリ使用し無限に保存する------------------------------------------------------------------------
		else if (key == 't'){
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
			//--------------------------------------------------------------------------
			cvShowImage("Camera", image1);
			D(key);
		}
		//設定枚数分の画像を保存する------------------------------------------------------------------------
		else if (key == 'S'){
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
		else if (key == 'T'){

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
		else if (key == 'm' || key == '4'){
			//マッチテンプレート
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
			sprintf_s(strR, "%s\\数値データ\\結果データ.csv", FolderName);
			if (error = fopen_s(&file, strR, "w") != 0){
				printf("%s\n", strR);
				fprintf(stderr, "結果データに書き込めません.処理を中断します.\n");
				fprintf(stderr, "（Excelなどで開いていると書き込めませんので閉じてください.）\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}
			else{ fclose(file); }

			//for csv
			int myFILECOUNT = fscanClock();
			if (myFILECOUNT == -1){
				fprintf(stderr, "撮影データがありません.\n");
				fprintf(stderr, "撮影を行ってください.\n\n");
				cvShowImage("Camera", image1);
				D('b');
				continue;
			}
			//--------------------------------------------------------------------------------------
			printf("%d個のテンプレートマッチング処理を行います。少々お待ちください。\n", myFILECOUNT);

			//読み込み
			sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);						//template picture as bmp format
			tmp_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);

			//読み込み失敗
			if (tmp_img == NULL) {
				fprintf(stderr, "テンプレート画像の読込みに失敗しました.\n");
				fprintf(stderr, "テンプレート画像を作成してください.\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}
			//read config.txt
			if (error = fopen_s(&file_config, "./実験フォルダ/Config.txt", "r") != 0){
				fprintf(stderr, "Config.txt の読込みに失敗しました.\n");
				fprintf(stderr, "再起動してください.\n\n");
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
					cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);				//methodも変えつつ検証が必要

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
						cvWaitKey(2);
					}
					else{
						// (2)テンプレートに対応する位置に矩形を描画、中心点も描画
						cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tmp_img->width, max_loc.y + tmp_img->height), CV_RGB(0, 0, 255), 2);//青枠四角を描画
						cvCircle(src_img, cvPoint(max_loc.x + tmp_img->width / 2, max_loc.y + tmp_img->height / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画
						//タイムスタンプ

						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//白縁
						cvPutText(src_img, sprintClockwPer(i, (int)(val4files[i] * 100.0)), cvPoint(20, 25), &font, CV_RGB(0, 0, 0));			//黒文字

						num_effected++;
						sprintf_s(strR, "%s\\処理画像\\Effected_%04d.bmp", FolderName, i);
						cvSaveImage(strR, src_img);
						cvShowImage("Camera", src_img);
						cvWaitKey(2);
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
			if (use_src_img){
				cvReleaseImage(&src_img);
			}
			cvReleaseImage(&tmp_img);
			//結果データの書き込み
			sprintf_s(strR, "%s\\数値データ\\結果データ.csv", FolderName);
			if (error = fopen_s(&file, strR, "w") != 0){
				printf("%s\n", strT);
				fprintf(stderr, "結果データに書き込めません.\n");
				fprintf(stderr, "（Excelなどで開いていると書き込めません.）\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}

			//----------------------------------------------------------------------------------------------
			//結果CSV1行目の記述
			//fprintf(file, "画像ファイル名,時,分,秒,ミリ秒,経過時間,t(s),x[pixel],y[pixel],X[meter],Y[meter],,ボールの直径(メートル)→,(ボールの直径を入力),テンプレート画像でのピクセルサイズ→,(テンプレート画像の横サイズを入力,1ピクセルあたりメートル出力→,=$N$1/$P$1\n");
			fprintf(file, "ボールの直径(メートル)→,,,,(ここに入力),\n" "テンプレート画像でのピクセルサイズ→,,,,(ここに入力)\n" "1ピクセルあたりメートル出力→,,,,=$J$1/$L$1\n" "類似度,%f\n" "-------,-------,-------,-------,-------,-------,-------,\n", config_val);
			fprintf(file, "画像ファイル名,経過時間,t(s),x[pixel],y[pixel],X[meter],Y[meter]\n", config_val);
			if (num_effected != 0){
				//各行の記述
				int c = 2; // line number of csv file (start from 2)
				for (i = 0; i < myFILECOUNT; i++){
					if (val4files[i] <config_val){
						//printf("%d skiped\n",i);
						//getchar();
					}
					//数値データにファイル名など座標、エクセル上での計算式を出力させる
					else{
						char FName[50];
						sprintf_s(FName, "img_%04d.bmp,%s", i, sprintClock(i));
						fprintf(file, FName);				//ファイル名&時刻データを出力
						//fprintf(file, ",= B%d *60*60*100 + C%d * 60 *1000 + D%d * 1000 + E%d", c, c, c, c);		//秒．ミリ秒座標を数値データ出力
						//fprintf(file, ",= (F%d - $F$2)/1000", c);		//１フレームごとの時間座標を数値データ出力
						fprintf(file, ",= B%d - $B$2", c);		//１フレームごとの時間座標を数値データ出力
						fprintf(file, XYpoint[i]);		//XY座標を数値データ出力
						//fprintf(file, ",=(H%d-$H$2)*$R$1,=(I%d-$I$2)*$R$1", c, c);		//xとyを数値データ出力
						fprintf(file, ",=(D%d-$D$2)*$N$1,=(E%d-$E$2)*$N$1", c, c);		//xとyを数値データ出力

						fprintf(file, "\n");			//改行しないと横1列になる
						c++;
					}
				}
			}
			fclose(file);

			printf("数値データ出力が終了しました.\n");

			//----------------------------------------------------------------------------------------------
			fprintf(stderr, "マッチング処理が終了しました。\n");


			printf("\n\n類似度%.1f%%以上の検出結果\n全%d枚中 %d枚検出できました.\n\n", config_val*100.0, num_bmp, num_effected);
			printf("---------検出数が少ない場合の対処---------\n");
			printf("※テンプレート画像やConfig.txtを変更し再度Match Templateの実行.\n");
			cvShowImage("Camera", image1);
			D(key);


		}
		else if (key == 'R'){
			fprintf(stderr, "マッチングのテストを行います.(Space key で終了).\n");
			IplImage *tmp_img;
			IplImage *tmp_img2;

			//テンプレート読み込み
			sprintf_s(strR, "%s\\テンプレート.bmp", FolderName);						//template picture as bmp format
			tmp_img = cvLoadImage(strR, CV_LOAD_IMAGE_COLOR);
			if (tmp_img == NULL) {//読み込み失敗
				fprintf(stderr, "テンプレート画像の読込みに失敗しました.\n");
				fprintf(stderr, "テンプレート画像を作成してください.\n\n");
				cvShowImage("Camera", image1);
				D('d');
				continue;
			}
			tmp_img2 = cvCreateImage(cvSize(tmp_img->width, tmp_img->height), tmp_img->depth, tmp_img->nChannels);
			cvCopy(tmp_img, tmp_img2);

			//read config.txt
			if (error = fopen_s(&file_config, "./実験フォルダ/Config.txt", "r") != 0){
				fprintf(stderr, "Config.txt の読込みに失敗しました.\n");
				fprintf(stderr, "再起動してください.\n\n");
				cvShowImage("Camera", image1);
				D('d');
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
					cvMatchTemplate(src_img, tmp_img, dst_img, CV_TM_CCOEFF_NORMED);				//定常追跡
				}
				else{
					cvMatchTemplate(src_img, tmp_img2, dst_img, CV_TM_CCOEFF_NORMED);				//動的追跡
				}
				cvMinMaxLoc(dst_img, &min_val, &max_val, &min_loc, &max_loc, NULL);
				cvReleaseImage(&dst_img);


				//動的追跡用に tmp_img2 を変更
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

				// (2)テンプレートに対応する位置に矩形を描画、中心点も描画
				CvScalar cvrgb1 = CV_RGB(0, 0, 255);
				CvScalar cvrgb2 = CV_RGB(0, 0, 0);
				if (max_val < config_val){//テンプレートと探索対象の類似度がconfig_val 未満
					cvrgb1 = CV_RGB(255, 0, 0);
					cvrgb2 = CV_RGB(200, 0, 0);
				}
				cvRectangle(src_img, max_loc, cvPoint(max_loc.x + tw, max_loc.y + th), cvrgb1, 2);//枠四角を描画
				cvCircle(src_img, cvPoint(max_loc.x + tw / 2, max_loc.y + th / 2), 1, CV_RGB(0, 255, 0), -1, 8);//中心描画
				cvPutText(src_img, sprintClockwPer(i, (int)(max_val * 100.0)), cvPoint(20, 25), &font_w, CV_RGB(255, 255, 255));	//白縁
				cvPutText(src_img, sprintClockwPer(i, (int)(max_val * 100.0)), cvPoint(20, 25), &font, cvrgb2);			//文字

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
				fprintf(stderr, "撮影データがありません.\n");
				fprintf(stderr, "撮影を行ってください.\n\n");
				D('b');
				continue;
			}
			printf("\n\n%d枚の撮影データを表示します.\n", myFILECOUNT);
			//--------------------------------------------------------------------------------------
			bool use_src_img = FALSE;
			for (i = 0; i < myFILECOUNT; i++){
				if (GetAsyncKeyState(VK_SPACE) & 0x8000){
					printf("Space keyが入力されました。記録を終了します。\n");
					break;
				}

				else if (key == 'V'){
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



/*順番通りに動かしてもらうためのキーボード説明表示*/
void D(int K)		//キー操作一覧のテキスト表示
{
	fprintf(stderr, "--------------------------------------------------------------------------\n");
	//fprintf(stderr, "※Cameraウィンドウをアクティブにすることでキー操作が可能\n\n");
	if (K == 'd' || K == 'D'){
		fprintf(stderr, "解析処理で必要となるテンプレート元画像の作成\n");
		fprintf(stderr, " b :テンプレート用画像の保存.\n");
		fprintf(stderr, " q or Esc : このアプリケーションを終了する\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else if (K == 'b' || K == 'B'){
		fprintf(stderr, "撮影保存方法\n");
		fprintf(stderr, " s :画像を250枚保存\n");
		fprintf(stderr, " q or Esc : このアプリケーションを終了する\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else if (K == 's' || K == 't' || K == 'S'){
		fprintf(stderr, "撮影が失敗した場合は再撮影操作を行ってください.\n");
		fprintf(stderr, " s :画像を250枚保存\n");


		fprintf(stderr, " 撮影成功した場合は次の動作を行ってください.\n");
		fprintf(stderr, " 解析処理、終了処理\n");
		fprintf(stderr, " m :Match Template　の実行,数値出力.\n");
		fprintf(stderr, " q or Esc : このアプリケーションを終了する\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else if (K == 'm' || K == 'M'){
		fprintf(stderr, " 解析処理を再実行、終了処理\n");
		fprintf(stderr, " m :Match Template　の実行,数値出力.\n");
		fprintf(stderr, " q or Esc : このアプリケーションを終了する\n");
		fprintf(stderr, "-------------------------------------------------------------------------\n");
	}
	else{
		fprintf(stderr, "キー入力(%c)の処理を終えました.次の処理を行ってください．\n", K);
		D0();
	}

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