#include <QApplication>
#include <QImage>
#include <QTextStream>
#include <QFile>
#include <iostream>
#include <QFileDialog>

// Функция преобразования изображения в ASCII-арт
void imageToAscii(const QImage& image, const QString& outputFileName, int widthScale, int heightScale)
{
    QFile file(outputFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Unable to open file for writing:" << outputFileName;
        return;
    }

    QTextStream stream(&file);

    int scaledWidth = image.width() / widthScale;
    int scaledHeight = image.height() / heightScale;

    // Проход по каждому блоку пикселей и преобразование в символ ASCII
    for (int y = 0; y < scaledHeight; ++y)
    {
        for (int x = 0; x < scaledWidth; ++x)
        {
            int averageGrayValue = 0;

            // Вычисление среднего значения яркости пикселей в блоке
            for (int dy = 0; dy < heightScale; ++dy)
            {
                for (int dx = 0; dx < widthScale; ++dx)
                {
                    QRgb pixelColor = image.pixel(x * widthScale + dx, y * heightScale + dy);
                    averageGrayValue += qGray(pixelColor);
                }
            }

            averageGrayValue /= (widthScale * heightScale);

            // Преобразование среднего значения в символ ASCII и запись в файл
            char asciiChar = "@%#*+=-:. "[averageGrayValue / 32];
            stream << asciiChar;
        }
        stream << "\n"; // Переход на новую строку после обработки строки
    }

    file.close();
}

// Функция восстановления изображения из ASCII-арта
QImage asciiToImage(const QString& asciiFileName, int widthScale, int heightScale)
{
    QFile file(asciiFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Unable to open file for reading:" << asciiFileName;
        return QImage(); // Возвращаем пустое изображение в случае ошибки
    }

    QTextStream stream(&file);

    // Вычисление размеров изображения на основе количества строк и символов в файле
    int imageWidth = 0;
    int imageHeight = 0;

    while (!stream.atEnd())
    {
        QString line = stream.readLine();
        imageWidth = qMax(imageWidth, line.length()) ;
        imageHeight++;
    }

    // Создание QImage с вычисленными размерами
    QImage image(imageWidth * widthScale, imageHeight * heightScale, QImage::Format_RGB32);
    image.fill(Qt::black); // Заполнение изображения белым фоном

    file.seek(0); // Сброс позиции файла в начало

    for (int y = 0; y < imageHeight; ++y)
    {
        QString line = stream.readLine();

        for (int x = 0; x < line.length(); ++x)
        {
            QChar character = line.at(x);

            int grayValue = character.toLatin1(); // Преобразование символа в его ASCII-значение
            grayValue *= 8; // Масштабирование значения для получения адекватной яркости

            /*
            // Получение индекса символа в строке символов ASCII
            int index = QString("@%#*+=-:. ").indexOf(character);
            // Масштабирование индекса символа для получения значения яркости
            int grayValue = index * 32;
            */

            // Установка цвета пикселя на основе масштабированного значения ASCII
            image.setPixel(x * widthScale, y * heightScale, qRgb(grayValue, grayValue, grayValue));
        }
    }

    file.close();

    return image;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Запрос выбора изображения
    QString inputImageFileName = QFileDialog::getOpenFileName(nullptr, "Select Image", QDir::homePath(), "Images (*.jpg)");
    QImage image(inputImageFileName);

    if (image.isNull())
    {
        qDebug() << "Unable to load image:" << inputImageFileName;
        return -1;
    }

    // Параметры для преобразования изображения в ASCII-арт
    QString outputTextFileName = "output.txt";
    int widthScale = 6;
    int heightScale = 12;

    // Преобразование изображения в ASCII-арт
    imageToAscii(image, outputTextFileName, widthScale, heightScale);
    qDebug() << "Conversion complete. ASCII art saved to:" << outputTextFileName;

    // Загрузка ASCII-арта и восстановление изображения
    QString inputAsciiFileName = "output.txt";
    QImage outputImage = asciiToImage(inputAsciiFileName, widthScale/3, heightScale/3);

    if (outputImage.isNull())
    {
        qDebug() << "Error converting ASCII to image.";
        return -1;
    }

    // Сохранение восстановленного изображения
    QString outputImageFileName = "output.jpg";
    outputImage.save(outputImageFileName, "jpg");
    qDebug() << "Conversion complete. Image saved to:" << outputImageFileName;

    return a.exec();
}
