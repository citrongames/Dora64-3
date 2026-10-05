# Oppi316–317 в составе блока2

В основной код добавлены оба варианта птицы: взлёт100, полёт101, круговой
полёт, death/fade, constructors/contacts и зависимые Source spawn/unlink.
Model clock использует Native cadence; waits/RNG/sounds и scene globals
выполняются один раз на исходный actor period.

Взлёт сохраняет yaw+5, pitch-2 до300 и рост вертикального scalar8. Пропорциональный
доворот flight101 использует исходный shortest angular delta к300 и ROM0.05:
continuous angle вместе с дискретным kick-before-drift projection продолжен
экспоненциально. Source h=1 сохраняет фактические proposal bits. Literal copies
heading/pitch используют соответствующие captured operands.

Height-target query phase0, conditional floor+150, initial setters, thresholds,
clip binds и failed births сохраняются как Source commands.12 функций получили
имена/описания и scoped pointer roles. Это новая непроверенная реализация;
generation/CPU tests/Windows build будут только после всего блока2.
