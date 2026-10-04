@echo off
setlocal
chcp 65001 >nul

REM ============================================
REM  Отправка репозитория без истории
REM  (один коммит + force push на GitHub)
REM ============================================

set "BRANCH=main"
set "TMPBRANCH=main_new"
set "REMOTE=origin"
set "COMMIT_MSG=Initial commit"

git rev-parse --is-inside-work-tree >nul 2>&1
if errorlevel 1 (
    echo [ОШИБКА] Текущая папка не git-репозиторий.
    pause
    exit /b 1
)

echo.
echo Отправляю ветку "%BRANCH%" на %REMOTE% без истории...
echo.

REM --- Удаляем временную ветку, если осталась от прошлого запуска ---
git show-ref --verify --quiet "refs/heads/%TMPBRANCH%"
if not errorlevel 1 git branch -D "%TMPBRANCH%"

REM --- Orphan-ветка: без родителей, без истории ---
git checkout --orphan "%TMPBRANCH%"
if errorlevel 1 goto :error

REM --- Индексируем всё (учитывая .gitignore) ---
git add -A
if errorlevel 1 goto :error

REM --- Единственный коммит ---
git commit -m "%COMMIT_MSG%"
if errorlevel 1 goto :error

REM --- Заменяем старую ветку ---
git branch -D "%BRANCH%"
if errorlevel 1 goto :error

git branch -m "%BRANCH%"
if errorlevel 1 goto :error

REM --- Force push ---
git push "%REMOTE%" "%BRANCH%" --force-with-lease
if errorlevel 1 goto :push_failed

echo.
echo ============================================
echo  ГОТОВО. Ветка "%BRANCH%" на GitHub —
echo  один коммит, без истории.
echo ============================================
echo.
pause
exit /b 0


:push_failed
echo.
echo [ОШИБКА] Push не удался.
echo.
echo Попробуйте:
echo   git fetch %REMOTE%
echo   git push %REMOTE% %BRANCH% --force-with-lease
echo.
echo Если ветка защищена — снимите защиту в
echo Settings -^> Branches на GitHub.
echo.
pause
exit /b 1


:error
echo.
echo [ОШИБКА] Что-то пошло не так. Проверьте: git status
echo Вернуться к старой истории: git reflog
echo.
pause
exit /b 1