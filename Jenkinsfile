pipeline {
    // Один агент на весь pipeline (у тебя это сейчас MacOS Jenkins)
    agent any

    options {
        timestamps()
        skipDefaultCheckout(true)
    }

    parameters {
        // Ветки для разных "ОС-пайплайнов"
        booleanParam(name: 'RUN_LINUX',  defaultValue: true,  description: 'Run Linux-style pipeline')
        booleanParam(name: 'RUN_MAC',    defaultValue: false, description: 'Run macOS-style pipeline')

        // Дополнительные галочки только для Linux-ветки
        booleanParam(name: 'RUN_COVERAGE',    defaultValue: false, description: 'Run coverage build (gcovr, Linux only)')
        booleanParam(name: 'RUN_ASAN',        defaultValue: false, description: 'Run AddressSanitizer build (Linux only)')
        booleanParam(name: 'RUN_UBSAN',       defaultValue: false, description: 'Run UndefinedBehaviorSanitizer build (Linux only)')
        booleanParam(name: 'RUN_TSAN',        defaultValue: false, description: 'Run ThreadSanitizer build (Linux only)')
        booleanParam(name: 'PACKAGE_RELEASE', defaultValue: false, description: 'Package release artifact after tests (Linux only)')
    }

    environment {
        CLANG_FORMAT_VERSION = '21'
    }

    stages {
        // ---------- COMMON ----------

        stage('Checkout') {
            steps {
                ansiColor('xterm') {
                    checkout scm
                    // Чтобы переиспользовать исходники в разных стадиях/матрицах
                    stash name: 'source', includes: '**/*'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[INFO] Checked out sources"
                        echo "[INFO] Running on OS: $(uname -s)"
                    '''
                }
            }
        }

        // ---------- LINUX-ВЕТКА (по сути просто «Linux-конфигурация», сейчас бежит на твоём Mac Jenkins) ----------

        stage('Bootstrap env (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[Linux] Bootstrap environment"
                        which meson  || echo "[WARN] meson not in PATH"
                        which ninja  || echo "[WARN] ninja not in PATH"
                        which gcovr || echo "[INFO] gcovr not found (coverage optional)"
                    '''
                }
            }
        }

        stage('Lint: clang-format (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[Linux] clang-format check"
                        if ! command -v clang-format-${CLANG_FORMAT_VERSION} >/dev/null 2>&1; then
                            echo "[WARN] clang-format-${CLANG_FORMAT_VERSION} not found, skipping"
                            exit 0
                        fi
                        # Подстрой под свою структуру
                        find src tests -type f \\( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \\) | while read -r file; do
                            clang-format-${CLANG_FORMAT_VERSION} --dry-run --Werror "$file"
                        done
                        echo "[Linux] clang-format OK"
                    '''
                }
            }
        }

        stage('Analyze: clang-tidy (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[Linux] clang-tidy analysis (demo)"

                        if ! command -v clang-tidy >/dev/null 2>&1; then
                            echo "[WARN] clang-tidy not found, skipping analysis"
                            exit 0
                        fi

                        # Простой пример через CMake, чтобы получить compile_commands.json
                        rm -rf build-tidy
                        mkdir -p build-tidy
                        cd build-tidy
                        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
                        cd ..

                        src_files=$(find src -name '*.cpp' || true)
                        if [ -z "$src_files" ]; then
                            echo "[WARN] No src/*.cpp files found for clang-tidy"
                            exit 0
                        fi

                        clang-tidy -p build-tidy $src_files
                    '''
                }
            }
        }

        stage('Build & Test (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            matrix {
                // Наследуем общий agent any
                axes {
                    axis {
                        name 'BUILD_TYPE'
                        values 'debug', 'release'
                    }
                }

                stages {
                    stage('Configure') {
                        steps {
                            ansiColor('xterm') {
                                unstash 'source'
                                sh '''
                                    #!/usr/bin/env bash
                                    set -euo pipefail
                                    export PATH="$HOME/.local/bin:$PATH"
                                    builddir="build-${BUILD_TYPE}"
                                    echo "[Linux][${BUILD_TYPE}] meson setup"
                                    rm -rf "${builddir}"
                                    meson setup "${builddir}" --buildtype=${BUILD_TYPE}
                                '''
                            }
                        }
                    }

                    stage('Build') {
                        steps {
                            ansiColor('xterm') {
                                sh '''
                                    #!/usr/bin/env bash
                                    set -euo pipefail
                                    export PATH="$HOME/.local/bin:$PATH"
                                    builddir="build-${BUILD_TYPE}"
                                    echo "[Linux][${BUILD_TYPE}] meson compile"
                                    meson compile -C "${builddir}"
                                '''
                            }
                        }
                    }

                    stage('Test') {
                        steps {
                            ansiColor('xterm') {
                                sh '''
                                    #!/usr/bin/env bash
                                    set -euo pipefail
                                    export PATH="$HOME/.local/bin:$PATH"
                                    builddir="build-${BUILD_TYPE}"
                                    echo "[Linux][${BUILD_TYPE}] meson test"
                                    meson test -C "${builddir}" --print-errorlogs
                                '''
                            }
                        }
                    }
                }
            }
        }

        stage('Coverage (gcovr, Linux)') {
            when {
                expression { params.RUN_LINUX && params.RUN_COVERAGE }
            }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="$HOME/.local/bin:$PATH"
                        builddir="build-coverage"
                        echo "[Linux][coverage] configure & build"
                        rm -rf "${builddir}"
                        meson setup "${builddir}" --buildtype=debug -Db_coverage=true
                        meson compile -C "${builddir}"

                        echo "[Linux][coverage] run tests"
                        meson test -C "${builddir}" --print-errorlogs

                        if command -v gcovr >/dev/null 2>&1; then
                            echo "[Linux][coverage] generate gcovr report"
                            gcovr -r . --xml-pretty -o "${builddir}/coverage.xml"
                        else
                            echo "[WARN] gcovr not found, skipping coverage XML"
                        fi
                    '''
                }
            }
        }

        stage('Sanitizers (Linux)') {
            when {
                expression { params.RUN_LINUX && (params.RUN_ASAN || params.RUN_UBSAN || params.RUN_TSAN) }
            }
            matrix {
                axes {
                    axis {
                        name 'SANITIZER'
                        values 'address', 'undefined', 'thread'
                    }
                }

                // Оставляем только те, что включены параметрами
                when {
                    expression {
                        (SANITIZER == 'address'   && params.RUN_ASAN)  ||
                        (SANITIZER == 'undefined' && params.RUN_UBSAN) ||
                        (SANITIZER == 'thread'    && params.RUN_TSAN)
                    }
                }

                stages {
                    stage('Configure/Build/Test') {
                        steps {
                            ansiColor('xterm') {
                                unstash 'source'
                                sh '''
                                    #!/usr/bin/env bash
                                    set -euo pipefail
                                    export PATH="$HOME/.local/bin:$PATH"
                                    builddir="build-${SANITIZER}"
                                    echo "[Linux][${SANITIZER}] configure & build"
                                    rm -rf "${builddir}"
                                    CC=clang CXX=clang++ meson setup "${builddir}" --buildtype=debug -Db_sanitize=${SANITIZER} -Db_lundef=false
                                    meson compile -C "${builddir}"

                                    echo "[Linux][${SANITIZER}] run tests"
                                    meson test -C "${builddir}" --print-errorlogs
                                '''
                            }
                        }
                    }
                }
            }
        }

        stage('Package (CD, Linux)') {
            when {
                expression { params.RUN_LINUX && params.PACKAGE_RELEASE }
            }
            steps {
                ansiColor('xterm') {
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="$HOME/.local/bin:$PATH"
                        builddir="build-release"
                        echo "[Linux][package] configure & build"
                        rm -rf "${builddir}"
                        meson setup "${builddir}" --buildtype=release
                        meson compile -C "${builddir}"

                        echo "[Linux][package] pack artifact"
                        tar czf artifact-linux.tar.gz -C "${builddir}" .
                    '''
                }
            }
        }

        // ---------- macOS-ВЕТКА (по сути второй набор конфигураций, сейчас тоже бежит на том же агенте) ----------

        stage('Bootstrap env (macOS)') {
            when {
                expression { params.RUN_MAC }
            }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[macOS] Bootstrap environment"
                        which meson || echo "[WARN] meson not in PATH (brew install meson ninja)"
                        which ninja || echo "[WARN] ninja not in PATH"
                    '''
                }
            }
        }

        stage('Build & Test (macOS)') {
            when {
                expression { params.RUN_MAC }
            }
            matrix {
                axes {
                    axis {
                        name 'BUILD_TYPE'
                        values 'debug', 'release'
                    }
                }

                stages {
                    stage('Configure') {
                        steps {
                            ansiColor('xterm') {
                                unstash 'source'
                                sh '''
                                    #!/usr/bin/env bash
                                    set -euo pipefail
                                    builddir="build-mac-${BUILD_TYPE}"
                                    echo "[macOS][${BUILD_TYPE}] meson setup"
                                    rm -rf "${builddir}"
                                    meson setup "${builddir}" --buildtype=${BUILD_TYPE}
                                '''
                            }
                        }
                    }

                    stage('Build') {
                        steps {
                            ansiColor('xterm') {
                                sh '''
                                    #!/usr/bin/env bash
                                    set -euo pipefail
                                    builddir="build-mac-${BUILD_TYPE}"
                                    echo "[macOS][${BUILD_TYPE}] meson compile"
                                    meson compile -C "${builddir}"
                                '''
                            }
                        }
                    }

                    stage('Test') {
                        steps {
                            ansiColor('xterm') {
                                sh '''
                                    #!/usr/bin/env bash
                                    set -euo pipefail
                                    builddir="build-mac-${BUILD_TYPE}"
                                    echo "[macOS][${BUILD_TYPE}] meson test"
                                    meson test -C "${builddir}" --print-errorlogs
                                '''
                            }
                        }
                    }
                }
            }
        }
    }

    post {
        always {
            // Собираем все JUnit-логи Meson'а (Linux + macOS)
            junit allowEmptyResults: true, testResults: '**/meson-logs/testlog.junit.xml'

            // Логи Meson'а
            archiveArtifacts artifacts: '**/meson-logs/**', allowEmptyArchive: true

            // Архивы с артефактами
            archiveArtifacts artifacts: 'artifact-*.tar.gz', allowEmptyArchive: true
        }

        failure {
            echo 'Build or tests failed. Check console log and meson-logs.'
        }
    }
}
