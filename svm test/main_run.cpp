#include <iostream>  
#include <fstream>  
#include <strstream>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/objdetect/objdetect.hpp>  
#include <opencv2/ml/ml.hpp>  
#include "someMethod.h"
#include "parameter.h"
using namespace std;  
using namespace cv;  

//-----------------------������----------------------------
//---------------------------------------------------------

int main()  
{
	//��������
    int descriptorDimDetect;//HOG�����ӵ�ά����[(��ⴰ�ڳ�-block��)/block����+1]*[(��ⴰ�ڸ�-block��)/block����+1]*bin����*(block��/cell��)*(block��/cell��)
	int descriptorDimClassify;
    MySVM detectSvm;//���SVM
	MySVM classifySvm;//����SVM
	detectSvm.load(DetectSvmName);
	classifySvm.load(ClassifySvmName);

	//----------------���л����˺��ϰ���ļ�������-----------------
	//---------------------------------------------------------------
	//��������
	HOGDescriptor detectHOG(WinSizeDetect,BlockSizeDetect,BlockStrideDetect,CellSizeDetect,NbinsDetect);//����HOG�����
	HOGDescriptor classifyHOG(WinSizeClassify,BlockSizeClassify,BlockStrideClassify,CellSizeClassify,NbinsClassify);//���HOG�����ڼ���������HOG��������
    descriptorDimDetect = detectSvm.get_var_count();//����������ά������HOG�����ӵ�ά������ǰ��ѵ��ʱ�Ĵ�Сһ������Ӵ˾���Ϊ���ڲ�ѵ��ʱҲ���õ�ά����
    descriptorDimClassify = classifySvm.get_var_count();//����������ά������HOG�����ӵ�ά������ǰ��ѵ��ʱ�Ĵ�Сһ������Ӵ˾���Ϊ���ڲ�ѵ��ʱҲ���õ�ά����
	int supportVectorDetectNum = detectSvm.get_support_vector_count();//֧�������ĸ���
	int supportVectorCassifyNum = classifySvm.get_support_vector_count();//֧�������ĸ���
    cout<<"Detect֧������������"<<supportVectorDetectNum<<endl;  
	cout<<"Cassify֧������������"<<supportVectorCassifyNum<<endl;  
    Mat alphaDetectMat = Mat::zeros(1, supportVectorDetectNum, CV_32FC1);//alpha���������ȵ���֧����������
	Mat alphaClassifyMat = Mat::zeros(1, supportVectorCassifyNum, CV_32FC1);//alpha���������ȵ���֧���������� 
    Mat supportVectorDetectMat = Mat::zeros(supportVectorDetectNum, descriptorDimDetect, CV_32FC1);//֧����������  
	Mat supportVectorClassifyMat = Mat::zeros(supportVectorCassifyNum, descriptorDimClassify, CV_32FC1);//֧����������  
    Mat resultDetectMat = Mat::zeros(1, descriptorDimDetect, CV_32FC1);//alpha��������֧����������Ľ��  
	Mat resultClassifyMat = Mat::zeros(1, descriptorDimClassify, CV_32FC1);//alpha��������֧����������Ľ��  

    //����w����
    for(int i=0; i<supportVectorDetectNum; i++)//��֧�����������ݸ��Ƶ�supportVectorMat������  
	{
        const float * pSVData = detectSvm.get_support_vector(i);//���ص�i��֧������������ָ��  
        for(int j=0; j<descriptorDimDetect; j++)  
            supportVectorDetectMat.at<float>(i,j) = pSVData[j];  
    }
	for(int i=0; i<supportVectorCassifyNum; i++)//��֧�����������ݸ��Ƶ�supportVectorMat������  
	{
		const float * pSVData = classifySvm.get_support_vector(i);//���ص�i��֧������������ָ��  
		for(int j=0; j<descriptorDimClassify; j++)  
			supportVectorClassifyMat.at<float>(i,j) = pSVData[j];  
	}
    double * pAlphaDetectData = detectSvm.get_alpha_vector();//����SVM�ľ��ߺ����е�alpha����  
	double * pAlphaClassifyData = classifySvm.get_alpha_vector();//����SVM�ľ��ߺ����е�alpha����  
    for(int i=0; i<supportVectorDetectNum; i++)//��alpha���������ݸ��Ƶ�alphaMat��  
        alphaDetectMat.at<float>(0,i) = pAlphaDetectData[i];  
	for(int i=0; i<supportVectorCassifyNum; i++)//��alpha���������ݸ��Ƶ�alphaMat��  
		alphaClassifyMat.at<float>(0,i) = pAlphaClassifyData[i];  
	resultDetectMat = -1 * alphaDetectMat * supportVectorDetectMat;//����-(alphaMat * supportVectorMat),����ŵ�resultMat��
    resultClassifyMat = -1 * alphaClassifyMat * supportVectorClassifyMat;//����-(alphaMat * supportVectorMat),����ŵ�resultMat�� 

    //�õ����յ�setSVMDetector(const vector<float>& detector)�����п��õļ����  
    vector<float> myDetector;//����Hog������SVM����ӣ�w+b��
    for(int i=0; i<descriptorDimDetect; i++)//��resultMat�е����ݸ��Ƶ�����myDetector��  
        myDetector.push_back(resultDetectMat.at<float>(0,i));  
    myDetector.push_back(detectSvm.get_rho());//������ƫ����rho���õ������  
    cout<<"����Hog������SVM�����ά��(w+b)��"<<myDetector.size()<<endl;

	//����SVMDetector�����
    detectHOG.setSVMDetector(myDetector);  


	//-------------------------������Ƶ���л����˼��-----------------------------------
	//��������
	VideoCapture myVideo(TestVideo);//��ȡ��Ƶ  
	Mat src,dst;					//ԭʼͼ�񣬴����ͼ��
	
	const char * sd1 = {"md "ResultVideoFile_1};//������ż���ͼ���ļ���
	system(sd1);
	const char * sd2 = {"md "ResultVideoFile_2};//������ż���ͼ���ļ���
	system(sd2);
	const char * sd3 = {"md "ResultVideoFile_3};//������ż���ͼ���ļ���
	system(sd3);

	//����Ƶ
	if(!myVideo.isOpened()){cout<<"��Ƶ��ȡ����"<<endl;system("puase");return -1;}

	//�������ɵ���Ƶ
	double videoRate=myVideo.get(CV_CAP_PROP_FPS);//��ȡ֡��
	int videoWidth=myVideo.get(CV_CAP_PROP_FRAME_WIDTH);//��ȡ��Ƶͼ����
	int videoHight=myVideo.get(CV_CAP_PROP_FRAME_HEIGHT);//��ȡ��Ƶͼ��߶�
	int videoDelay=1000/videoRate;//ÿ֮֡����ӳ�����Ƶ��֡�����Ӧ�������ܳ����ʱ�򲥷���Ƶ�����ʣ�
	VideoWriter outputVideo(ResultVideo, CV_FOURCC('M', 'J', 'P', 'G'), videoRate, Size(videoWidth, videoHight));//������Ƶ��

	//��ʼ��Ƶ����
	bool stop = false;
	for (int fnum = 1;!stop;fnum++)
	{
		cout<<fnum<<endl;
		//��������
		if (!myVideo.read(src)){cout<<"��Ƶ����"<<endl;waitKey(0); break;}//��ȡ��Ƶ֡
		//resize(videoFrame,videoFrame,Size(0,0),2,2);//������Ƶͼ��Ĵ�С
		src.copyTo(dst);
		vector<Rect> found;//��������

		//��ͼƬ���ж�߶Ȼ����˼�� 
		cout<<"���ж�߶ȼ��"<<endl;
		detectHOG.detectMultiScale(src, found, HitThreshold, WinStride, Size(0,0), DetScale, 2, false);
		//������1Դͼ��2���������3���������ͳ�ƽ��ľ���4�ƶ�����(������block������������)5��Ե��չ6Դͼ��ͼ��ÿ����С����7�������8���෽ʽ

		//�Լ�⵽��ͼ����з���
		for(int i=0; i<found.size(); i++)  
		{
			cout<<"width:"<<found[i].width<<"  height:"<<found[i].height<<endl;//�������ͼ��С
			vector<float> descriptors;//HOG����������
			Mat descriptorsMat = Mat::zeros(1, descriptorDimClassify, CV_32FC1);//�����õ�HOG����������������=1������=��������ά��
			Mat temp;
			resize(src(found[i]),temp,WinSizeClassify);//���������ͼ��ߴ�

			classifyHOG.compute(temp,descriptors);//����HOG������
			for(int i=0; i<descriptorDimClassify; i++)  
				descriptorsMat.at<float>(0,i) = descriptors[i];//������������ֵ

			float classifyResult = classifySvm.predict(descriptorsMat);//���������ͼ����Ԥ��

			if (classifyResult == 1)//������
			{
				rectangle(dst, found[i], Scalar(255,0,0), 3);//��ͼ�л�������
				if (SAVESET)//�Ƿ񱣴�������
				{
					strstream ss;
					string s;
					ss<<ResultVideoFile_1<<1000*fnum+i<<".jpg";
					ss>>s;
					imwrite(s,src(found[i]));
				}
			} 
			else if (classifyResult == 2)//�ϰ���
			{
				rectangle(dst, found[i], Scalar(0,255,0), 3);//��ͼ�л�������
				if (SAVESET)//�Ƿ񱣴�������
				{
					strstream ss;
					string s;
					ss<<ResultVideoFile_2<<1000*fnum+i<<".jpg";
					ss>>s;
					imwrite(s,src(found[i]));
				}
			}
			else if (classifyResult ==3)//����
			{
				rectangle(dst, found[i], Scalar(0,0,255), 3);//��ͼ�л�������
				if (SAVESET)//�Ƿ񱣴�������
				{
					strstream ss;
					string s;
					ss<<ResultVideoFile_3<<1000*fnum+i<<".jpg";
					ss>>s;
					imwrite(s,src(found[i]));
				}
			}
			else//����
			{
				rectangle(dst, found[i], Scalar(255,255,255), 3);//��ͼ�л�������
			}
		}

		//������Ƶͼ��
		outputVideo<<dst;
		imshow("dst",dst);
		if(waitKey(1)>=0)stop = true;//ͨ������ֹͣ��Ƶ
	}
	//---------------------------------end---------------------------------------
}  