pipeline {
    agent any

    options {
        timestamps()
        skipDefaultCheckout(true)
    }

    parameters {
        booleanParam(name: 'RUN_LINUX',  defaultValue: false, description: 'Run Linux-style pipeline')
        booleanParam(name: 'RUN_MAC',    defaultValue: true,  description: 'Run macOS pipeline')

        booleanParam(name: 'RUN_COVERAGE',    defaultValue: false, description: 'Run coverage build (gcovr, Linux only)')
        booleanParam(name: 'RUN_ASAN',        defaultValue: false, description: 'Run AddressSanitizer build (Linux only)')
        booleanParam(name: 'RUN_UBSAN',       defaultValue: false, description: 'Run UndefinedBehaviorSanitizer build (Linux only)')
        booleanParam(name: 'RUN_TSAN',        defaultValue: false, description: 'Run ThreadSanitizer build (Linux only)')
        booleanParam(name: 'PACKAGE_RELEASE', defaultValue: false, description: 'Package release artifact after tests')
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
                    stash name: 'source', includes: '**/*'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        echo "[INFO] Checked out sources"
                        echo "[INFO] Running on OS: $(uname -s)"
                        echo "[INFO] PATH in Jenkins: $PATH"
                    '''
                }
            }
        }

        // ---------- LINUX-ВЕТКА (конфигурация под Linux, сейчас тоже крутится на этом же ноде) ----------

        stage('Bootstrap env (Linux)') {
            when { expression { params.RUN_LINUX } }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        # добавляем brew-пути, но НЕ выкидываем старый PATH
                        export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
                        echo "[Linux] Bootstrap environment"
                        which meson  || echo "[WARN] meson not in PATH"
                        which ninja  || echo "[WARN] ninja not in PATH"
                        which gcovr || echo "[INFO] gcovr not found (coverage optional)"
                    '''
                }
            }
        }

        stage('Lint: clang-format (Linux)') {
            when { expression { params.RUN_LINUX } }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
                        echo "[Linux] clang-format check"
                        if ! command -v clang-format-21 >/dev/null 2>&1; then
                            echo "[WARN] clang-format-21 not found, skipping"
                            exit 0
                        fi
                        find src tests -type f \\( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \\) | while read -r file; do
                            clang-format-21 --dry-run --Werror "$file"
                        done
                        echo "[Linux] clang-format OK"
                    '''
                }
            }
        }

        stage('Analyze: clang-tidy (Linux)') {
            when { expression { params.RUN_LINUX } }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
                        echo "[Linux] clang-tidy analysis"

                        if ! command -v clang-tidy >/dev/null 2>&1; then
                            echo "[WARN] clang-tidy not found, skipping analysis"
                            exit 0
                        fi

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
            when { expression { params.RUN_LINUX } }
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
                                    export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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
                                    export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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
                                    export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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
            when { expression { params.RUN_LINUX && params.RUN_COVERAGE } }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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
            when { expression { params.RUN_LINUX && (params.RUN_ASAN || params.RUN_UBSAN || params.RUN_TSAN) } }
            matrix {
                axes {
                    axis {
                        name 'SANITIZER'
                        values 'address', 'undefined', 'thread'
                    }
                }

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
                                    export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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
            when { expression { params.RUN_LINUX && params.PACKAGE_RELEASE } }
            steps {
                ansiColor('xterm') {
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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

        // ---------- macOS-ВЕТКА ----------

        stage('Bootstrap env (macOS)') {
            when { expression { params.RUN_MAC } }
            steps {
                ansiColor('xterm') {
                    unstash 'source'
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
                        echo "[macOS] Bootstrap environment"
                        which meson || echo "[WARN] meson not in PATH (brew install meson ninja)"
                        which ninja || echo "[WARN] ninja not in PATH"
                    '''
                }
            }
        }

        stage('Build & Test (macOS)') {
            when { expression { params.RUN_MAC } }
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
                                    export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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
                                    export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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
                                    export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
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

        stage('Package (macOS)') {
            when { expression { params.RUN_MAC && params.PACKAGE_RELEASE } }
            steps {
                ansiColor('xterm') {
                    sh '''
                        #!/usr/bin/env bash
                        set -euo pipefail
                        export PATH="/opt/homebrew/bin:/usr/local/bin:$HOME/.local/bin:$PATH"
                        builddir="build-mac-release"
                        echo "[macOS][package] pack artifact"
                        if [ ! -d "${builddir}" ]; then
                            echo "[ERROR] ${builddir} does not exist. Make sure release build succeeded."
                            exit 1
                        fi
                        tar czf artifact-mac.tar.gz -C "${builddir}" .
                    '''
                }
            }
        }
    }

    post {
        always {
            junit allowEmptyResults: true, testResults: '**/meson-logs/testlog.junit.xml'
            archiveArtifacts artifacts: '**/meson-logs/**', allowEmptyArchive: true
            archiveArtifacts artifacts: 'artifact-*.tar.gz', allowEmptyArchive: true
        }

        failure {
            echo 'Build or tests failed. Check console log and meson-logs.'
        }
    }
}
