// CarTagReader by 'Neotod' and 'Hamfimay'
// with console GUI

/*
    ==> we use pictures in 24bit bmp format for processing, you can easily drop your picture file into a Hex Editor app, then see the magic of bmp pictures :))))
    ==> for more information about header of .bmp files, check this amazing and old site: http://turrier.fr/tutoriels/form_02/create-a-bitmap-file-with-an-hexadecimal-editor.html this was like a treasure :))
    ==> for more information about RGB to Grayscale, check this site: https://www.tutorialspoint.com/dip/grayscale_to_rgb_conversion.htm
    ==> our image processing method:  1-get the cartag picture in specefic size (size can be changed with other c++ libraries), this code won't resize or clarify the cartag picture (you can see example cartags
        in folder 'examples').  2-convert picture into boolean matrix for easier processing  3-crop each number or letter in the cartag picture  4-resize each cropped part of picture according to our base pictures
        5-compare each cropped part with our base pictures(letters and numbers) and get percent of similarity for knowing how much they are similar  6-if that percent was mroe than a specific number,
         that cropped part is character of that base picture.    finished, EzPz!
    ==> our resize method only will make cartag picture smaller, so, base pictures must have smaller size than cartag picture.
    ==> if want to get better result from this code, you can use your own base pictures in bigger sizes. but, you should have better processor for more speed :))
    ==> we assume our cartag have 3 letters, one dash and 4 numbers.
    ==> you have to use a specific font both in your cartag and your base pictures. we use 'fe_font' in this project.
    ==> resize algorithm is by 'hamfimay', if you don't understand it even with comments, blame her :))
*/

#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

using namespace std;

class Image
{
    private:
        vector < vector<bool> > picMatrix; // number of rows: height of picture, number of columns: width of picture

        // two vectors bellow are related to cropping picMatrix
        // NOTE: there is an array in widthEdgePositions matrix for each elements of heightEdgePositions array
        vector<int> heightEdgePositions; // these picMatrix positions are for horizontal cropping
        vector< vector<int> > widthEdgePositions; // these picMatrix positions are for vertical cropping
        string carTag;

        struct numbers // for storing base number pictures in zeros and ones(boolean)
        {
            vector < vector<bool> > matrix;
        } baseNumber[10];

        struct alphabets // for storing base letter pictures in zeros and ones(boolean)
        {
            vector < vector<bool> > matrix;
            string letter;
        } baseLetter[27];

        static int charToInt(char c) // TODO: get ascii code of character
        {
            for (int i = 0; i < 256; i++)
                if(c == char(i))
                    return i;
        }

        static int getNumberOfZeros(int lineWidth) // TODO: getting number of additional zeros in each line in HexEditor, for understanding this, you should read the link in line: 6
        {
            int number = ((4 - (lineWidth * 3) % 4) % 4);
            return number;
        }

        static int getWidth(ifstream& image)
        {
            char header[54];
            image.seekg(0, ios::beg);
            image.read(header, 54);
            int width = 0;
            for(int i = 21; i > 17; i--) // TODO: convert number in base 256 to base 10
                width = width * 256 + charToInt(header[i]);
            return width;
        }

        static int getHeight(ifstream& image)
        {
            char header[54];
            image.seekg(0, ios::beg);
            image.read(header, 54);
            int height = 0;
            for(int i = 25; i > 21; i--) // TODO: convert number in base 256 to base 10
            {
                height = height * 256 + charToInt(header[i]);
            }
            return height;
        }

        static void setImageMatrix(ifstream& image, vector< vector<bool> > &matrix) // TODO: convert picture into ones and zeros (boolean) matrix
        {
            int width = getWidth(image);
            int height = getHeight(image);
            int zeros = getNumberOfZeros(width);
            int lineCharsNumber = width * 3 + zeros;
            char pixelsLine[lineCharsNumber];

            int end = width*3;
            vector <bool> tempVector;
            for(int i = 0; i < height; i++)
            {
                image.read(pixelsLine, lineCharsNumber);

                tempVector.clear();
                for (int j = 0; j < end; j+=3)
                {
                    int average = charToInt(pixelsLine[j]) * 0.11 + charToInt(pixelsLine[j+1]) * 0.59 // check line: 7 for more information
                    + charToInt(pixelsLine[j+2]) * 0.3;

                    if(average < 70) // we got this number (70) with trial and error in paint :))
                        tempVector.push_back(true);
                    else
                        tempVector.push_back(false);
                }
                matrix.push_back(tempVector);
            }
        }

        bool isThereAnyTrueInOneRow(vector< vector<bool> > &matrix, int numOfRow)
        {
            int end = matrix[numOfRow].size() - 1;
            for(int i = 0; i <= end; i++)
                if(matrix[numOfRow][i] == true)
                    return true;
            return false;
        }

        bool isThereAnyTrueInOneColumn(vector< vector<bool> > &matrix, vector<int> &heightPositions, int numOfColumn)
        {
            int size = heightPositions.size();
            for(int i = heightPositions[size-2]; i <= heightPositions[size-1]; i++)
                if(matrix[i][numOfColumn] == true)
                    return true;
            return false;
        }

        void getCroppingPositions(vector< vector<bool> > &matrix, vector<int> &heightPositions, vector< vector<int> > &widthPositions)
        {
            for(int i = 0; i < matrix.size(); i++)
            {
                if(isThereAnyTrueInOneRow(matrix, i) == true)
                {
                    heightPositions.push_back(i);
                    while(isThereAnyTrueInOneRow(matrix, i) == true && i < matrix.size())
                        i++;
                    heightPositions.push_back(i - 1);

                    widthPositions.push_back({});

                    // until here, height positions for horizontal cropping are specified, now let's specify width positions for vertical cropping
                    int numberOfRows = (heightPositions.size() / 2) - 1;
                    for(int j = 0; j < matrix[0].size(); j++)
                    {
                        if(isThereAnyTrueInOneColumn(matrix, heightPositions, j) == true)
                        {
                            widthPositions[numberOfRows].push_back(j);
                            while(isThereAnyTrueInOneColumn(matrix, heightPositions, j) == true && j < matrix[0].size())
                                j++;
                            widthPositions[numberOfRows].push_back(j - 1);
                        }
                    }
                }
            }
        }

        int getHeightTarget(vector< vector<bool> > &matrix)
        {
            int returnValue = matrix.size();
            return returnValue;
        }

        int getWidthTarget(vector< vector<bool> > &matrix)
        {
            int returnValue = matrix[0].size();
            return returnValue;
        }

        void resizeHeightOfPic(vector< vector<bool> > &matrix, int targetSize)
        {
            // this while is for halving matrix height till it's possible
            // for doing this => nth element (n < matrix.size/2) = (2n)th element | (2n+1)th element
            // I use "|" for saving true elements :|

            while(matrix.size() >= targetSize * 2)
            {
                for(int i = 0; i < matrix.size() - 1; i += 2)
                    for(int j = 0; j < matrix[i].size(); j++)
                        matrix[i / 2][j] = matrix[i][j] | matrix[i + 1][j];

                int primarySize = matrix.size();
                for(int i = 0; i < primarySize - ((primarySize - 1) / 2); i++)
                    matrix.pop_back();
            }
            int difference = matrix.size() - targetSize;
            if(difference)
            {
                int n = targetSize / difference;

                // *elements = the elements which we save their value(if they were true) in their right neighbor
                // then transfer it (like shift) to the element which is before the last *element in the end of matrix height

                // I want to some elements as *elements uniformly (something like this: *--*--*--)
                // for this I reach another *elements after n element

                for(int i = 0; i < difference; i++) // this is for saving value of *elements (if they are true) in their right neighbor
                    for(int j = 0; j < matrix[0].size(); j++)
                        matrix[i * (n + 1 ) + 1][j] = matrix[i * (n + 1)][j] | matrix[i * (n + 1 ) + 1][j];

                for(int i = 0; i < difference; i++) // this is for transferring *elements to the element which is before the last *element in the end of matrix height
                    for(int j = i * n; j < matrix.size() - (i + 1); j++)
                        for(int k = 0; k < matrix[0].size(); k++)
                        {
                            matrix[j][k] = matrix[j + 1][k] ^ matrix[j][k];
                            matrix[j + 1][k] = matrix[j][k] ^ matrix[j + 1][k];
                            matrix[j][k] = matrix[j + 1][k] ^ matrix[j][k];
                        }

                for(int i = 0; i < difference; i++) // this is for removing *elements
                    matrix.pop_back();
            }
        }

        void resizeWidthOfPic(vector< vector<bool> > &matrix, int targetSize)
        {
            // this while is for halving matrix width till it's possible
            // for doing this ==> nth element (n < matrix[0].size/2) = (2n)th element | (2n+1)th element
            // I use "|" for saving true elements :|

            while(matrix[0].size() >= targetSize * 2)
            {
                for(int i = 0; i < matrix.size() ; i++)
                    for(int j = 0; j < matrix[0].size() - 1; j++)
                        matrix[i][j / 2] = matrix[i][j] | matrix[i][j + 1];

                int primarySize = matrix[0].size();
                for(int i = 0; i < matrix.size(); i++)
                    for(int j = 0; j < primarySize - ((primarySize - 1) / 2) ; j++)
                        matrix[i].pop_back();
            }
            int difference = matrix[0].size() - targetSize;
            if(difference)
            {
                // *elements = the elements which we save their value(if they were true) in their right neighbor
                // then transfer it (like shift) to the element which is before the last *element in the end of matrix height

                // I want to some elements as *elements uniformly (something like this: *--*--*--)
                // for this I reach another *elements after n element
                int n = targetSize / difference;

                for(int i = 0; i < matrix.size(); i++) // this is for saving value of *elements (if they are true) in their right neighbor
                    for(int j = 0; j < difference; j++)
                        matrix[i][j * (n + 1) + 1] = matrix[i][j * (n + 1) + 1] | matrix[i][j * (n + 1)];

                for(int i = 0; i < matrix.size(); i++) // this is for transferring *elements to the element which is before the last *element in the end of matrix width
                    for(int j = 0; j < difference; j++)
                        for(int k = j * n; k < matrix[0].size() - (j + 1); k++)
                        {
                            matrix[i][k] = matrix[i][k + 1] ^ matrix[i][k];
                            matrix[i][k + 1] = matrix[i][k] ^ matrix[i][k + 1];
                            matrix[i][k] = matrix[i][k + 1] ^ matrix[i][k];
                        }

                for(int i = 0; i < matrix.size(); i++) // this is for removing *elements
                    for(int j = 0; j < difference; j++)
                        matrix[i].pop_back();
            }
        }

        void resizeMatrix(vector< vector<bool> > &matrix, int widthTarget, int heightTarget)
        {
            resizeHeightOfPic(matrix, heightTarget);
            resizeWidthOfPic(matrix, widthTarget);
        }

        float getPercentageOfSimilarity(vector < vector<bool> > &matrix, vector < vector<bool> > &baseMatrix)
        {
            float similarityNumber = 0;
            for(int i = 0; i < matrix.size(); i++)
            {
                for(int j = 0; j < matrix[i].size(); j++)
                {
                    if(matrix[i][j] == 1 && baseMatrix[i][j] == 1 || matrix[i][j] == 0 && baseMatrix[i][j] == 0)
                        similarityNumber++;
                }
            }
            float pixelsNumber = baseMatrix.size() * baseMatrix[0].size();
            float percentage = (similarityNumber / pixelsNumber)*100 ;
            return percentage;
        }

        void checkSimilarityOfTwoNumbers(vector < vector<bool> > &matrix, bool isNumber, bool isLetter) // two numbers: cropped character from car tag picture and base pictures
        {
            int maxPercentage = 0;

            if(isNumber == true)
            {
                int bestNumber;
                for(int i = 0; i < 10; i++)
                {
                    vector < vector<bool> > copyMatrix; // because of not letting our matrix get resized
                    copyMatrix = matrix;
                    int heightTarget = getHeightTarget(baseNumber[i].matrix);
                    int widthTarget = getWidthTarget(baseNumber[i].matrix);
                    resizeMatrix(copyMatrix, widthTarget, heightTarget);
                    float percentage = getPercentageOfSimilarity(copyMatrix, baseNumber[i].matrix);

                    if(percentage > maxPercentage)
                    {
                        bestNumber = i;
                        maxPercentage = percentage;
                    }
                }
                char character = bestNumber + 48;
                carTag.push_back(character);
            }

            if(isLetter == true)
            {
                string bestLetter;
                maxPercentage = 0;
                for(int i = 0; i < 27; i++)
                {
                    vector < vector<bool> > copyMatrix; // because of not letting our matrix get resized
                    copyMatrix = matrix;
                    int heightTarget = getHeightTarget(baseLetter[i].matrix);
                    int widthTarget = getWidthTarget(baseLetter[i].matrix);
                    resizeMatrix(copyMatrix, widthTarget, heightTarget);
                    float percentage = getPercentageOfSimilarity(copyMatrix, baseLetter[i].matrix);

                    if(percentage > maxPercentage)
                    {
                        bestLetter = baseLetter[i].letter;
                        maxPercentage = percentage;
                    }
                }
                carTag.append(bestLetter);
            }

        }

        void showLoadingBar(int processNumber)
        {
            processNumber = processNumber * 125;
            vector <char> loadingBar;
            float percentage = ((float)processNumber / 1000) * 100;
            for(int i = 0; i < processNumber; i++)
            {
                loadingBar.push_back('=');
            }
            for(int i = processNumber; i < 1000; i++)
            {
                loadingBar.push_back(' ');
            }

            cout << "\r\t\t\tProcessing [";
            for(int i = 0; i < loadingBar.size(); i+=10)
            {
                cout << loadingBar[i];
            }
            cout << "]  /  " << (int)percentage << "%";
            Sleep(500);

            if(processNumber == 1000) // for make out loading bar blinking, after the process finished
            {
                for(int i = 0; i < 5; i++)
                {
                    cout << "\r\t\t\t";
                    for(int j = 0; j < loadingBar.size() - 15; j+=8)
                    {
                        cout << " ";
                    }
                    Sleep(200);

                    cout << "\r\t\t\tProcessing [";
                    for(int j = 0; j < loadingBar.size(); j+=10)
                    {
                        cout << loadingBar[j];
                    }
                    cout << "]  /  " << 100 << "%";
                    Sleep(200);
                }
                Sleep(500);
            }

        }

        void giveMeTheDamnCarTag(vector < vector<bool> > &matrix) // TODO: it's obvious :))
        {
            SetConsoleTitle(TEXT("Processing..."));

            getCroppingPositions(matrix, heightEdgePositions, widthEdgePositions); // cropping positions will be stored in heightEdgePositions and widthEdgePositions vectors

            vector < vector<bool> > croppedNumberMatrix;
            vector <bool> rowVector;
            int size = widthEdgePositions[0].size();

            int counter = 0; // will be used for showing loading bar, it's will be increased one value after reading a character from car tag
            int m = 0;
            cout << endl;

            showLoadingBar(counter);
            // sending each cropped part of car tag picture for getting the character
            for(int k = 0; k < widthEdgePositions[0].size()/2; k++)
            {

                for(int i = heightEdgePositions[0]; i <= heightEdgePositions[1]; i++)
                {
                    for(int j = widthEdgePositions[0][m]; j <= widthEdgePositions[0][m+1]; j++)
                    {
                        rowVector.push_back(matrix[i][j]);
                    }
                    croppedNumberMatrix.push_back(rowVector);
                    rowVector.clear();
                }

                cropBasePicture(croppedNumberMatrix); // for final cropping :)
                if(counter < 3) // cropped matrix character is letter
                {
                    checkSimilarityOfTwoNumbers(croppedNumberMatrix, false, true);
                }
                else if(counter == 3) // character is dash character
                {
                    carTag.append("-");
                }
                else if(counter > 3) // cropped matrix character is number
                {
                    checkSimilarityOfTwoNumbers(croppedNumberMatrix, true, false);
                }

                m+=2;
                counter++;
                croppedNumberMatrix.clear();
                showLoadingBar(counter);
            }
        }

        void cropBasePicture(vector < vector<bool> > &matrix) // TODO: cropping a matrix of a picture that have just one character (like base pictures)
        {
            vector <int> numberHeightEdges;
            vector < vector<int> > numberWidthEdges;

            getCroppingPositions(matrix, numberHeightEdges, numberWidthEdges);

            vector < vector<bool> > croppedMatrix;
            vector <bool> rowVector;

            for(int i = numberHeightEdges[0]; i <= numberHeightEdges[1]; i++)
            {
                for(int j = numberWidthEdges[0][0]; j <= numberWidthEdges[0][1]; j++)
                {
                    rowVector.push_back(matrix[i][j]);
                }
                croppedMatrix.push_back(rowVector);
                rowVector.clear();
            }

            //swap croppedMatrix and matrix
            matrix.clear();
            rowVector.clear();

            matrix = croppedMatrix;
        }

        void setBasePicsMatrixes()
        {
            ifstream zeroPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\0.bmp", ios::in | ios::binary);
            setImageMatrix(zeroPic, baseNumber[0].matrix);

            ifstream onePic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\1.bmp", ios::in | ios::binary);
            setImageMatrix(onePic, baseNumber[1].matrix);

            ifstream twoPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\2.bmp", ios::in | ios::binary);
            setImageMatrix(twoPic, baseNumber[2].matrix);

            ifstream threePic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\3.bmp", ios::in | ios::binary);
            setImageMatrix(threePic, baseNumber[3].matrix);

            ifstream fourPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\4.bmp", ios::in | ios::binary);
            setImageMatrix(fourPic, baseNumber[4].matrix);

            ifstream fivePic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\5.bmp", ios::in | ios::binary);
            setImageMatrix(fivePic, baseNumber[5].matrix);

            ifstream sixPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\6.bmp", ios::in | ios::binary);
            setImageMatrix(sixPic, baseNumber[6].matrix);

            ifstream sevenPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\7.bmp", ios::in | ios::binary);
            setImageMatrix(sevenPic, baseNumber[7].matrix);

            ifstream eightPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\8.bmp", ios::in | ios::binary);
            setImageMatrix(eightPic, baseNumber[8].matrix);

            ifstream ninePic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\9.bmp", ios::in | ios::binary);
            setImageMatrix(ninePic, baseNumber[9].matrix);

            for(int i = 0; i < 10; i++)
            {
                cropBasePicture(baseNumber[i].matrix);
            }

            ifstream APic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\A.bmp", ios::in | ios::binary);
            setImageMatrix(APic, baseLetter[0].matrix);
            baseLetter[0].letter = "A";

            ifstream BPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\B.bmp", ios::in | ios::binary);
            setImageMatrix(BPic, baseLetter[1].matrix);
            baseLetter[1].letter = "B";

            ifstream CPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\C.bmp", ios::in | ios::binary);
            setImageMatrix(CPic, baseLetter[2].matrix);
            baseLetter[2].letter = "C";

            ifstream DPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\D.bmp", ios::in | ios::binary);
            setImageMatrix(DPic, baseLetter[3].matrix);
            baseLetter[3].letter = "D";

            ifstream EPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\E.bmp", ios::in | ios::binary);
            setImageMatrix(EPic, baseLetter[4].matrix);
            baseLetter[4].letter = "E";

            ifstream FPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\F.bmp", ios::in | ios::binary);
            setImageMatrix(FPic, baseLetter[5].matrix);
            baseLetter[5].letter = "F";

            ifstream GPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\G.bmp", ios::in | ios::binary);
            setImageMatrix(GPic, baseLetter[6].matrix);
            baseLetter[6].letter = "G";

            ifstream HPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\H.bmp", ios::in | ios::binary);
            setImageMatrix(HPic, baseLetter[7].matrix);
            baseLetter[7].letter = "H";

            ifstream IPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\I.bmp", ios::in | ios::binary);
            setImageMatrix(IPic, baseLetter[8].matrix);
            baseLetter[8].letter = "I";

            ifstream JPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\J.bmp", ios::in | ios::binary);
            setImageMatrix(JPic, baseLetter[9].matrix);
            baseLetter[9].letter = "J";

            ifstream KPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\K.bmp", ios::in | ios::binary);
            setImageMatrix(KPic, baseLetter[10].matrix);
            baseLetter[10].letter = "K";

            ifstream LPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\L.bmp", ios::in | ios::binary);
            setImageMatrix(LPic, baseLetter[11].matrix);
            baseLetter[11].letter = "L";

            ifstream MPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\M.bmp", ios::in | ios::binary);
            setImageMatrix(MPic, baseLetter[12].matrix);
            baseLetter[12].letter = "M";

            ifstream NPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\N.bmp", ios::in | ios::binary);
            setImageMatrix(NPic, baseLetter[13].matrix);
            baseLetter[13].letter = "N";

            ifstream OPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\O.bmp", ios::in | ios::binary);
            setImageMatrix(OPic, baseLetter[14].matrix);
            baseLetter[14].letter = "O";

            ifstream PPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\P.bmp", ios::in | ios::binary);
            setImageMatrix(PPic, baseLetter[15].matrix);
            baseLetter[15].letter = "P";

            ifstream QPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\Q.bmp", ios::in | ios::binary);
            setImageMatrix(QPic, baseLetter[16].matrix);
            baseLetter[16].letter = "Q";

            ifstream RPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\R.bmp", ios::in | ios::binary);
            setImageMatrix(RPic, baseLetter[17].matrix);
            baseLetter[17].letter = "R";

            ifstream SPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\S.bmp", ios::in | ios::binary);
            setImageMatrix(SPic, baseLetter[18].matrix);
            baseLetter[18].letter = "S";

            ifstream TPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\T.bmp", ios::in | ios::binary);
            setImageMatrix(TPic, baseLetter[19].matrix);
            baseLetter[19].letter = "T";

            ifstream UPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\U.bmp", ios::in | ios::binary);
            setImageMatrix(UPic, baseLetter[20].matrix);
            baseLetter[20].letter = "U";

            ifstream VPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\V.bmp", ios::in | ios::binary);
            setImageMatrix(VPic, baseLetter[21].matrix);
            baseLetter[21].letter = "V";

            ifstream WPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\W.bmp", ios::in | ios::binary);
            setImageMatrix(WPic, baseLetter[22].matrix);
            baseLetter[22].letter = "W";

            ifstream XPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\X.bmp", ios::in | ios::binary);
            setImageMatrix(XPic, baseLetter[23].matrix);
            baseLetter[23].letter = "X";

            ifstream YPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\Y.bmp", ios::in | ios::binary);
            setImageMatrix(YPic, baseLetter[24].matrix);
            baseLetter[24].letter = "Y";

            ifstream ZPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\Z.bmp", ios::in | ios::binary);
            setImageMatrix(ZPic, baseLetter[25].matrix);
            baseLetter[25].letter = "Z";

            ifstream dashPic("F:\\Projects\\C++\\-[Project1]Image_Processing\\Images\\fe_font\\dash.bmp", ios::in | ios::binary);
            setImageMatrix(dashPic, baseLetter[26].matrix);
            baseLetter[26].letter = "-";

            for(int i = 0; i < 27; i++)
            {
                cropBasePicture(baseLetter[i].matrix);
            }
        }

        void showMatrix(vector < vector<bool> > &matrix)
        {
            for(int i = matrix.size()-1; i >= 0; i--)
            {
                cout << "    ";
                for(int j = 0; j < matrix[i].size(); j++)
                {
                    cout << matrix[i][j];
                }
                cout << endl;
            }
        }

        void showMinimizedTag()
        {
            string text = "\r\n\n\t\t\t\t\t\t\tThis is your minimized CarTag";
            string showingText = "";

            for(int i = 0; i < text.size(); i++)
            {
                system("CLS");
                showingText.push_back(text[i]);
                cout << showingText;
                Sleep(1);
            }
            Sleep(2000);

            cout << "\n\n";

            vector < vector<bool> > copyMatrix;
            copyMatrix = picMatrix;
            resizeMatrix(copyMatrix, 160, 40);

            showMatrix(copyMatrix);
            Sleep(3000);
            system("CLS");

            cout << "\r";
            showMatrix(copyMatrix);
        }

        void showCarTag()
        {
            string showingString;
            cout << "\n\n\n\t\t\t\t\t\t========================================================\n";
            cout << "\r\t\t\t\t\t\t\t\t\t" <<  carTag;
            cout << "\n\t\t\t\t\t\t========================================================";
            cout << "\n\n\n\n";
            system("PAUSE");
            SetConsoleTitle(TEXT("CarTagReader"));
        }

    public:
        void processImage(ifstream& image)
        {
            SetConsoleTitle(TEXT("CarTagReader"));

            setImageMatrix(image, picMatrix);
            setBasePicsMatrixes();
            showMinimizedTag();
            giveMeTheDamnCarTag(picMatrix);
            showCarTag();
        }

        void static sayWelcome()
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);

            string text = "\n\n\n\n\n\t\t\tWelcome to CarTagReader";
            string showingText = "";

            for(int i = 0; i < text.size(); i++)
            {
                system("CLS");
                showingText.push_back(text[i]);
                cout << showingText;
                Sleep(0.5);
            }
            Sleep(3000);

            text = "\n\n\n\n\n\t\t\tPlease choose your .bmp picture";
            showingText = "";
            for(int i = 0; i < text.size(); i++)
            {
                system("CLS");
                showingText.push_back(text[i]);
                cout << showingText;
                Sleep(0.5);
            }
            Sleep(2000);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
        }
};

int main()
{
    Image::sayWelcome();

    //these lines of code are used for browsing a picture from a dialog box
    char filename[ MAX_PATH ];

    OPENFILENAME ofn;
    ZeroMemory( &filename, sizeof( filename ) );
    ZeroMemory( &ofn,      sizeof( ofn ) );
    ofn.lStructSize  = sizeof( ofn );
    ofn.hwndOwner    = NULL;
    ofn.lpstrFilter  = ".bmp Files\0*.bmp\0Any File\0*.*\0\0";
    ofn.lpstrFile    = filename;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrTitle   = "Select a File";
    ofn.Flags        = OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA( &ofn ))
    {
        ifstream pic1(filename, ios::in | ios::binary);
        if(!pic1.fail())
        {
            Image image1;
            image1.processImage(pic1);
        }
    }


}
