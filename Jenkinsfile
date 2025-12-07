pipeline {
    // Агент по умолчанию не используется: каждый stage сам выбирает ноду
    agent none

    options {
        timestamps()
        skipDefaultCheckout(true)
    }

    parameters {
        // Выбор ОС пользователем
        booleanParam(name: 'RUN_LINUX',  defaultValue: false,  description: 'Run Linux pipeline')
        booleanParam(name: 'RUN_MAC',    defaultValue: true, description: 'Run macOS pipeline')

        // Дополнительные галочки только для Linux
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
        // ----------- COMMON -----------

        stage('Checkout') {
            agent any
            steps {
                ansiColor('xterm') {
                    checkout scm
                    // Чтобы можно было использовать исходники в нескольких стадиях / матрицах
                    stash name: 'source', includes: '**/*'
                }
            }
        }

        // ----------- LINUX STAGES -----------

        stage('Bootstrap env (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            agent { label 'linux' }
            steps {
                ansiColor('xterm') {
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[Linux] Bootstrap environment"
                        which meson  || echo "meson must be installed on Linux node"
                        which ninja  || echo "ninja must be installed on Linux node"
                        which gcovr || echo "gcovr (for coverage) is optional"
                    '''
                }
            }
        }

        stage('Lint: clang-format (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            agent { label 'linux' }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[Linux] clang-format check"
                        # При необходимости поправь директории/маски файлов
                        find src tests -name '*.cpp' -o -name '*.hpp' -o -name '*.h' | while read -r file; do
                            clang-format-${CLANG_FORMAT_VERSION} --dry-run --Werror "$file"
                        done
                    '''
                }
            }
        }

        stage('Analyze: clang-tidy (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            agent { label 'linux' }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[Linux] clang-tidy analysis"

                        # Простейший пример через CMake, если тебе нужен compile_commands.json
                        mkdir -p build-tidy
                        cd build-tidy
                        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
                        cd ..

                        clang-tidy -p build-tidy $(find src -name '*.cpp')
                    '''
                }
            }
        }

        stage('Build & Test (Linux)') {
            when {
                expression { params.RUN_LINUX }
            }
            matrix {
                // Вся матрица выполняется на linux-ноде (обычно master/built-in)
                agent { label 'linux' }

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
            agent { label 'linux' }
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

                        echo "[Linux][coverage] generate gcovr report"
                        gcovr -r . --xml-pretty -o "${builddir}/coverage.xml"
                    '''
                }
            }
        }

        stage('Sanitizers (Linux)') {
            when {
                expression { params.RUN_LINUX && (params.RUN_ASAN || params.RUN_UBSAN || params.RUN_TSAN) }
            }
            matrix {
                agent { label 'linux' }

                axes {
                    axis {
                        name 'SANITIZER'
                        values 'address', 'undefined', 'thread'
                    }
                }

                // Оставляем только те санитайзеры, которые включены параметрами
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
            agent { label 'linux' }
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

        // ----------- macOS STAGES -----------

        stage('Bootstrap env (macOS)') {
            when {
                expression { params.RUN_MAC }
            }
            agent { label 'mac' }
            steps {
                ansiColor('xterm') {
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[macOS] Bootstrap environment"
                        which meson || echo "meson must be installed on macOS node (e.g. brew install meson ninja)"
                        which ninja || echo "ninja must be installed on macOS node"
                    '''
                }
            }
        }

        stage('Build & Test (macOS)') {
            when {
                expression { params.RUN_MAC }
            }
            matrix {
                agent { label 'mac' }

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
            // Собираем все JUnit-логи Meson'а
            junit allowEmptyResults: true, testResults: '**/meson-logs/testlog.junit.xml'

            // Логи Meson'а на все случаи
            archiveArtifacts artifacts: '**/meson-logs/**', allowEmptyArchive: true

            // Упакованные релизные артефакты (Linux)
            archiveArtifacts artifacts: 'artifact-*.tar.gz', allowEmptyArchive: true
        }

        failure {
            echo 'Build or tests failed. Check console log and meson-logs.'
        }
    }
}
