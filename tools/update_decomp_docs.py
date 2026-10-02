"""Keep reviewed game function/variable names and documentation in sync."""
import argparse
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[1]
FUNCTION_DOC='docs/decomp-symbols.md'
VARIABLE_DOC='docs/decomp-variable-roles.md'

def documents(catalog):
    entries=catalog['functions']
    renamed=sum(e['name']!=e['legacy_name'] for e in entries)
    roles=sum(len(e.get('locals',[])) for e in entries)
    functions='''# Имена разобранного исходника игры

Источник истины — `recomp/decomp-symbols.json`, а не ручные правки файлов в
`build-local`. Генератор применяет каталог после рекомпиляции и проверяет ROM,
overlay, размер/хэш функции и границы ролей регистров. Одинаковый RAM адрес
в разных overlays не означает одинаковую функцию.

В генерируемом C определения и вызовы получают смысловые имена. Header
`RecompiledFuncs/decomp_names.h` раскрывает их в прежние экспортированные
имена: C ABI, адресные таблицы, function pointers, native hooks и старые отчёты
сохраняют соответствие. Это читаемые псевдонимы, а не восстановление оригинальных
имён разработчиков. Название отражает подтверждённую роль, а не полноту игрового
прохождения или готовность variable FPS.

Проверка `tools/check_decomp_names.py --generated <generation>` сравнивает
полный C token stream после preprocessing и выполняет C17 syntax compilation.
Генератор также требует точного восстановления исходного C до переименований.
ROM инструкции, типы, float порядок, register storage и control flow сохраняются.

После каждого нового разбора добавляй подтверждённое имя, описание, evidence,
полную ROM identity и только доказанные области переменных. Неизвестные роли
оставляй без смыслового переименования; уточняй каталог при новых находках.
`tools/update_decomp_docs.py --write` обновляет этот справочник, переменные,
address-map и ссылки на старые полные имена в остальных документах.
`--check` проверяет, что документация соответствует каталогу.

'''
    functions+=f'В каталоге: {len(entries)} описаний функций, {renamed} новых смысловых имён и {roles} ограниченных областей переменных.\n\n'
    functions+='| Overlay / адрес | Читаемое имя | Прежнее ABI имя | Назначение |\n|---|---|---|---|\n'
    for e in entries:
        desc=e['description'].replace('|','\\|')
        functions+=f'| {e["section"]} / {e["vram"]} | `{e["name"]}` | `{e["legacy_name"]}` | {desc} |\n'
    variables='''# Роли переменных в декомпилированном C

Регистр MIPS не равен одной исходной переменной: он может сменить роль в той же
функции. Каталог назначает имя точному доступу `ctx->rN` только в подтверждённой
области инструкций `[start, end)`. Входящий регистр, сохранённый caller state,
новый аргумент вызова и восстановление регистра остаются за этой областью.

Генерируемые локальные псевдонимы раскрываются в те же register lvalues.
Запись по понятному имени обновляет прежний слот контекста; новых копий значений
нет. Имена очищаются после функции. Точные интервалы, инструкции и основания
хранятся в `recomp/decomp-symbols.json` и выводятся рядом с определением функции.
Для неизвестных значений сохраняется регистровое имя.

Например, в полном player update `controller_index_r16` означает индекс
контроллера. В функции изменения здоровья `health_delta_r4` — входная прибавка
к здоровью до переиспользования r4 как status pointer. Эти роли нельзя переносить
на весь регистр или на другой overlay.

| Имя | Исходный доступ | Подтверждённая роль | Число областей |\n|---|---|---|---|\n'''
    grouped={}
    for e in entries:
        for role in e.get('locals',[]):
            key=(role['name'],role['expression'],role['description'])
            grouped[key]=grouped.get(key,0)+1
    for (name,expr,desc),count in sorted(grouped.items()):
        variables+=f'| `{name}` | `{expr}` | {desc.replace("|", " ")} | {count} |\n'
    return {FUNCTION_DOC:functions,VARIABLE_DOC:variables}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    mode=p.add_mutually_exclusive_group()
    mode.add_argument('--write',action='store_true');mode.add_argument('--check',action='store_true')
    a=p.parse_args();catalog=json.loads((ROOT/'recomp/decomp-symbols.json').read_text())
    mapping={e['legacy_name']:e['name'] for e in catalog['functions'] if e['name']!=e['legacy_name']}
    replacements=re.compile(r'\b(?:'+'|'.join(map(re.escape,mapping))+r')\b') if mapping else None
    expected=documents(catalog)
    for path in sorted((ROOT/'docs').iterdir()):
        name=str(path.relative_to(ROOT))
        if path.suffix not in ('.md','.csv') or name in expected: continue
        old=path.read_text()
        new=replacements.sub(lambda m:mapping[m[0]],old) if replacements else old
        if new!=old: expected[name]=new
    stale=[n for n,t in expected.items() if not (ROOT/n).exists() or (ROOT/n).read_text()!=t]
    if a.write:
        for n in stale: (ROOT/n).write_text(expected[n])
        print('Updated decomp documentation:',', '.join(stale) or 'already current')
    elif stale:
        raise SystemExit('Decomp documentation needs updating: '+', '.join(stale))
    else:
        print('PASS: reviewed function names and variable roles match documentation')

if __name__=='__main__': main()
