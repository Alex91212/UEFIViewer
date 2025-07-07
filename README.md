# UEFI Dump Viewer

Кроссплатформенный просмотрщик дампов UEFI с HEX-редактором

## Сборка и запуск

1. Установите зависимости:
   - Qt6 (Core, Widgets)
   - CMake ≥ 3.16
   - Компилятор C++17

2. Клонируйте с подмодулями:
```bash
git clone --recursive https://github.com/Alex91212/UEFIViewer.git
cd UEFIViewer
```
2,5. Соберите проект UEFITool для своей OS из исходников:
- Вот тут по-хорошему надо бы описать как это делать под разные OS
- Положите испольняемый файл UEFIExtract.exe (Windows) или UEFIExtract (mac/linux) по 
адресу: папка_проекта/tools/UEFIExtract/

3. Соберите проект:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

4. Запустите:
```bash
./UEFIViewer  # или UEFIViewer.exe на Windows
```

## Особенности
- Встроенный HEX-редактор
- Поддержка UEFIExtract для всех платформ
- Автоматическое определение типа файлов
