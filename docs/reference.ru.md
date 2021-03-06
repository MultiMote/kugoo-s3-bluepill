- [1. Краткие имена кнопок](#1-краткие-имена-кнопок)
- [2. Неочевидные комбинации клавиш](#2-неочевидные-комбинации-клавиш)
  - [2.1 Выключение](#21-выключение)
  - [2.2 Активация круиз-контроля](#22-активация-круиз-контроля)
  - [2.3 Сброс настроек](#23-сброс-настроек)
  - [2.4 Ограничение тока](#24-ограничение-тока)
- [3. Редактирование значений в меню настроек](#3-редактирование-значений-в-меню-настроек)
  - [3.1 Логические значения (1|0)](#31-логические-значения-10)
  - [3.2 Числовые значения](#32-числовые-значения)
- [4. Пункты меню настроек](#4-пункты-меню-настроек)
  - [ОТКЛЮЧИТЬ ЗВУКИ (1|0)](#отключить-звуки-10)
  - [ГРОМКОСТЬ КНОПОК](#громкость-кнопок)
  - [ГРОМКОСТЬ СИГНАЛОВ](#громкость-сигналов)
  - [АНТИ-ФИКСАЦИЯ УСКОР. (1|0)](#анти-фиксация-ускор-10)
  - [ZERO-START (1|0)](#zero-start-10)
  - [ПЛАВНОЕ УСКОРЕНИЕ (1|0)](#плавное-ускорение-10)
  - [ИНКРЕМ. ПЛАВН. УСКОР](#инкрем-плавн-ускор)
  - [ОГРАНИЧЕНИЕ ТОКА (1|0)](#ограничение-тока-10)
  - [ЗНАЧ. ОГРАНИЧЕН. ТОКА](#знач-ограничен-тока)
  - [КОЭФФ. СТАБ. ТОКА](#коэфф-стаб-тока)
  - [СТАБИЛИЗАЦИЯ СКОР. (1|0)](#стабилизация-скор-10)
  - [КОЭФФ. СТАБ. СКОРОСТИ](#коэфф-стаб-скорости)
  - [КАЛИБРОВКА РУЧЕК](#калибровка-ручек)
  - [АВТОКАЛИБРОВКА РУЧЕК (1|0)](#автокалибровка-ручек-10)
  - [СОХРАНЯТЬ СКОРОСТЬ](#сохранять-скорость)
  - [ДЛИНА ОКР. КОЛЕСА, ММ](#длина-окр-колеса-мм)
  - [КОЛИЧЕСТВО МАГНИТОВ](#количество-магнитов)
  - [ПОСЛЕДНИЕ ПОЕЗДКИ](#последние-поездки)


# 1. Краткие имена кнопок

`BEEP` - звуковой сигнал / возврат

`SET` - меню настроек / уменьшить значение

`POWER` - питание / подтверждения

`LIGHT` - фонарь / увеличить значение

`SPEED` - ограничение скорости


# 2. Неочевидные комбинации клавиш

## 2.1 Выключение

Выключение производится из главного экрана долгим нажатием кнопки `POWER`.

## 2.2 Активация круиз-контроля

Вывести ручку акселератора в ненулевое положение и нажать кнопку `SET`. Для
поддержания скорости используется значение ручки (не текущая скорость самоката).

Автоматической активации пока нет. И нужно ли?

## 2.3 Сброс настроек

При включении зажать `BEEP`. Далее нажать `LIGHT` чтобы сбросить все сохранённые
данные, или `SET`, чтобы сохранить при этом показания одометра.
Для отмены нажать `POWER`.

## 2.4 Ограничение тока

Вывести ручку тормоза в ненулевое положение и нажать кнопку `SPEED`.


# 3. Редактирование значений в меню настроек

  ## 3.1 Логические значения (1|0)

  Выбрать пункт меню и нажать кнопку питания.

  `✔` - включено

  `▪` - выключено

  ## 3.2 Числовые значения

  Выбрать пункт меню и нажать кнопку питания. Навигация между разрядами
  происходит с помощью кнопки питания. Увеличить значение разряда - кнопка
  `LIGHT`, уменьшить - кнопка `SET`. Для установки значения по-умолчанию
  используется кнопка `SPEED`. Выход из редактирования осуществляется
  кнопкой `BEEP`.

  При редактирования значений с плавающей точкой, возможны странные явления
  в связи с своеобразностью этого типа данных.

# 4. Пункты меню настроек

  ## ОТКЛЮЧИТЬ ЗВУКИ (1|0)

  Отключить все звуки кроме звука на кнопке сигнала.

  ## ГРОМКОСТЬ КНОПОК

  Громкость сигнала нажатия на кнопки.

  ## ГРОМКОСТЬ СИГНАЛОВ

  Громкость различных оповещений (например, вход-выход из круиз-контроля).
  Громкость "гудка" не регулируется этим параметром.

  ## АНТИ-ФИКСАЦИЯ УСКОР. (1|0)

  Подавление фиксации ускорения.

  Головная боль штатного контроллера мотор-колеса. Он же "круиз-контроль", коим
  на самом деле не является. Не отключается. При долгом удержании (около 5 секунд)
  ручки акселератора в одном положении её значение фиксируется, после чего ручку можно
  отпускать. При отключении данной функции в меню данное устройство совершает
  попытки подавить данный функционал методом периодической подачи нулевого
  ускорения на контроллер мотор-колеса.

  > ⚠️ При выключении данной функции становится невозможной какая-либо
  > продолжительная регулировка ускорения (стабилизация скорости, ограничение тока).

  ## ZERO-START (1|0)

  Двигатель начинает работать только после того, как начать его вращать.

  > ⚠️ При использовании стабилизации скорости и функции zero-start
  > гарантированы неожиданные результаты.

  ## ПЛАВНОЕ УСКОРЕНИЕ (1|0)

  Включение функции плавного старта. При повышении значения ручки акселератора,
  фактическое значение линейно нарастает. Хорошо работает вместе
  со стабилизацией скорости.

  ## ИНКРЕМ. ПЛАВН. УСКОР

  Инкремент плавного ускорения (км/ч).

  При включенной функции плавного ускорения к текущему значению скорости каждые
  50 мс добавляется данное значение до достижения требуемого.

  ## ОГРАНИЧЕНИЕ ТОКА (1|0)

  Включить или выключить ограничение тока.

  ## ЗНАЧ. ОГРАНИЧЕН. ТОКА

  Значение ограничения тока.

  При превышении заданного значения устройство совершает попытки снизить скорость
  движения, пока не понизится ток до доступного предела.

  ## КОЭФФ. СТАБ. ТОКА

  Коэффициент стабилизации тока.

  При превышении заданного значения разница тока умножается
  на этот коэффициент и отнимается от значения скорости (км/ч).
  Происходит это после приёма пакета от контроллера мотор-колеса.

  При установке значения в 0 стабилизация тока работать не может.

  ## СТАБИЛИЗАЦИЯ СКОР. (1|0)

  Включить или выключить стабилизацию скорости.

  Как это работает? При нажатии на ручку акселератора на контроллер мотор-колеса
  подаётся приблизительное значение ускорения для достижения данной скорости на
  холостых оборотах. Затем реальная скорость сравнивается с ожидаемой, разница
  значений умножается на коэффициент стабилизации скорости и прибавляется
  к передаваемому значению. Происходит это после приёма
  пакета от контроллера мотор-колеса.

  ## КОЭФФ. СТАБ. СКОРОСТИ

  Коэффициент стабилизации скорости.

  При установке значения в 0 стабилизация скорости работать не может.

  ## КАЛИБРОВКА РУЧЕК

  Произвести установку минимальных и максимальных значений ручек тормоза/акселератора.
  Калибровка происходит в следующем порядке:

  1. Нулевое положение ручки акселератора
  2. Максимальное положение ручки акселератора
  3. Нулевое положение ручки тормоза
  4. Максимальное положение ручки тормоза

  ## АВТОКАЛИБРОВКА РУЧЕК (1|0)

  При запуске устройства установить текущие положения ручек как минимальные.
  Минимальные значения ручной калибровки при этом игнорируются.

  ## СОХРАНЯТЬ СКОРОСТЬ

  Сохранять ли выбранное значение ограничения скорости после перезапуска.

  ## ДЛИНА ОКР. КОЛЕСА, ММ

  Длина окружности колеса в миллиметрах.

  Формула: `2 × π × (диаметр_в_мм / 2)`

  ## КОЛИЧЕСТВО МАГНИТОВ

  Количество магнитов в мотор-колесе. По умолчанию 30.

  ## ПОСЛЕДНИЕ ПОЕЗДКИ

  Открывает список последних восьми поездок. Данные отображаются в формате

  ```
  Время - Расстояние
  ```

  При долгом нажатии кнопки `SPEED` список очищается.
  Список обновляется при выключении устройства кнопкой `POWER`.
