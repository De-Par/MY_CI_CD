pipeline {
    agent any

    options {
        timestamps()
        ansiColor('xterm')
        skipDefaultCheckout(true)
    }

    parameters {
        booleanParam(name: 'RUN_LINUX', defaultValue: false, description: 'Запускать Linux-пайплайн')
        booleanParam(name: 'RUN_MAC', defaultValue: true, description: 'Запускать macOS-пайплайн')
        booleanParam(name: 'RUN_COVERAGE', defaultValue: false, description: 'Run coverage build (gcovr, Linux)')
        booleanParam(name: 'RUN_ASAN', defaultValue: false, description: 'Run AddressSanitizer build (Linux)')
        booleanParam(name: 'RUN_UBSAN', defaultValue: false, description: 'Run UndefinedBehaviorSanitizer build (Linux)')
        booleanParam(name: 'RUN_TSAN', defaultValue: false, description: 'Run ThreadSanitizer build (Linux)')
        booleanParam(name: 'PACKAGE_RELEASE', defaultValue: false, description: 'Package release artifact after tests (Linux)')
    }

    environment {
        CLANG_FORMAT_VERSION = '21'
    }

    stages {
        stage('Checkout') {
            agent any
            steps {
                checkout scm
                stash name: 'source', includes: '**/*'
            }
        }

        /* Linux блоки */
        stage('Bootstrap env (Linux)') {
            when { expression { params.RUN_LINUX } }
            agent { label 'linux' }
            steps {
                unstash 'source'
                sh '''
                    #!/usr/bin/env bash
                    set -euo pipefail
                    sudo apt-get update
                    sudo apt-get install -y python3-pip clang clang-tidy ninja-build pkg-config wget gnupg

                    # clang-format конкретной версии
                    wget https://apt.llvm.org/llvm.sh
                    chmod +x llvm.sh
                    sudo ./llvm.sh ${CLANG_FORMAT_VERSION}
                    sudo apt-get install -y "clang-format-${CLANG_FORMAT_VERSION}"

                    python3 -m pip install --user --upgrade pip
                    python3 -m pip install --user meson ninja gcovr

                    export PATH="$HOME/.local/bin:$PATH"
                    mkdir -p subprojects
                    if [ ! -f subprojects/gtest.wrap ]; then
                        meson wrap install gtest
                    fi
                '''
            }
        }

        stage('Lint: clang-format (Linux)') {
            when { expression { params.RUN_LINUX } }
            agent { label 'linux' }
            steps {
                unstash 'source'
                sh '''
                    #!/usr/bin/env bash
                    set -euo pipefail
                    export PATH="$HOME/.local/bin:$PATH"
                    files=$(find src include app tests -name '*.cpp' -o -name '*.hpp')
                    if [ -n "$files" ]; then
                        clang-format-${CLANG_FORMAT_VERSION} --dry-run --Werror $files
                    fi
                '''
            }
        }

        stage('Analyze: clang-tidy (Linux)') {
            when { expression { params.RUN_LINUX } }
            agent { label 'linux' }
            steps {
                unstash 'source'
                sh '''
                    #!/usr/bin/env bash
                    set -euo pipefail
                    export PATH="$HOME/.local/bin:$PATH"
                    rm -rf build-tidy
                    CC=clang CXX=clang++ meson setup build-tidy --buildtype=debug --backend=ninja
                    meson compile -C build-tidy
                    files=$(find src include app tests -name '*.cpp')
                    if [ -n "$files" ]; then
                        for f in $files; do
                            echo "==> clang-tidy $f"
                            clang-tidy "$f" -p build-tidy --warnings-as-errors='*'
                        done
                    fi
                '''
            }
        }

        stage('Build & Test (Linux)') {
            when { expression { params.RUN_LINUX } }
            agent { label 'linux' }
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
                            unstash 'source'
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                export PATH="$HOME/.local/bin:$PATH"
                                builddir="build-${BUILD_TYPE}"
                                rm -rf "${builddir}"
                                meson setup "${builddir}" --buildtype=${BUILD_TYPE}
                            '''
                        }
                    }

                    stage('Build') {
                        steps {
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                export PATH="$HOME/.local/bin:$PATH"
                                meson compile -C "build-${BUILD_TYPE}"
                            '''
                        }
                    }

                    stage('Test') {
                        steps {
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                export PATH="$HOME/.local/bin:$PATH"
                                meson test -C "build-${BUILD_TYPE}" --print-errorlogs
                            '''
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
                unstash 'source'
                sh '''
                    #!/usr/bin/env bash
                    set -euo pipefail
                    export PATH="$HOME/.local/bin:$PATH"
                    rm -rf build-coverage coverage.xml coverage.html
                    meson setup build-coverage --buildtype=debug -Db_coverage=true
                    meson compile -C build-coverage
                    meson test -C build-coverage --print-errorlogs
                    gcovr -r . --xml-pretty -o coverage.xml --html-details coverage.html || true
                '''
            }
            post {
                always {
                    archiveArtifacts artifacts: 'coverage.*', allowEmptyArchive: true
                }
            }
        }

        stage('Sanitizers (Linux)') {
            when {
                expression { params.RUN_LINUX && (params.RUN_ASAN || params.RUN_UBSAN || params.RUN_TSAN) }
            }
            agent { label 'linux' }
            matrix {
                axes {
                    axis {
                        name 'SANITIZER'
                        values 'address', 'undefined', 'thread'
                    }
                }

                when {
                    expression {
                        (SANITIZER == 'address' && params.RUN_ASAN) ||
                        (SANITIZER == 'undefined' && params.RUN_UBSAN) ||
                        (SANITIZER == 'thread' && params.RUN_TSAN)
                    }
                }

                stages {
                    stage('Configure/Build/Test') {
                        steps {
                            unstash 'source'
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                export PATH="$HOME/.local/bin:$PATH"
                                builddir="build-${SANITIZER}"
                                rm -rf "${builddir}"
                                CC=clang CXX=clang++ meson setup "${builddir}" --buildtype=debug -Db_sanitize=${SANITIZER} -Db_lundef=false
                                meson compile -C "${builddir}"
                                meson test -C "${builddir}" --print-errorlogs
                            '''
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
                unstash 'source'
                sh '''
                    #!/usr/bin/env bash
                    set -euo pipefail
                    export PATH="$HOME/.local/bin:$PATH"
                    rm -rf build-release install-root dist
                    meson setup build-release --buildtype=release
                    meson compile -C build-release
                    meson test -C build-release --print-errorlogs
                    meson install -C build-release --destdir=install-root
                    PKG_NAME="awesome_calc-${BUILD_TAG:-jenkins}-${BUILD_ID:-local}-linux.tar.gz"
                    mkdir -p dist
                    tar czf "dist/${PKG_NAME}" -C install-root .
                    echo "PACKAGE_NAME=${PKG_NAME}" > dist/package.txt
                '''
            }
            post {
                success {
                    archiveArtifacts artifacts: 'dist/**', fingerprint: true, allowEmptyArchive: false
                }
            }
        }

        /* macOS блоки */
        stage('Build & Test (macOS)') {
            when { expression { params.RUN_MAC } }
            matrix {
                agent { label 'mac' }
                axes {
                    axis {
                        name 'BUILD_TYPE'
                        values 'debug', 'release'
                    }
                }

                stages {
                    stage('Bootstrap macOS env') {
                        steps {
                            unstash 'source'
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                # Предполагается установленный Homebrew
                                brew update
                                brew install ninja meson llvm || true
                                python3 -m pip install --upgrade pip
                                python3 -m pip install --user meson ninja
                                export PATH="$HOME/.local/bin:$PATH"
                                mkdir -p subprojects
                                if [ ! -f subprojects/gtest.wrap ]; then
                                    meson wrap install gtest
                                fi
                            '''
                        }
                    }

                    stage('Configure') {
                        steps {
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                export PATH="$HOME/.local/bin:/opt/homebrew/opt/llvm/bin:$PATH"
                                builddir="mac-build-${BUILD_TYPE}"
                                rm -rf "${builddir}"
                                meson setup "${builddir}" --buildtype=${BUILD_TYPE}
                            '''
                        }
                    }

                    stage('Build') {
                        steps {
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                export PATH="$HOME/.local/bin:/opt/homebrew/opt/llvm/bin:$PATH"
                                meson compile -C "mac-build-${BUILD_TYPE}"
                            '''
                        }
                    }

                    stage('Test') {
                        steps {
                            sh '''
                                #!/usr/bin/env bash
                                set -euo pipefail
                                export PATH="$HOME/.local/bin:/opt/homebrew/opt/llvm/bin:$PATH"
                                meson test -C "mac-build-${BUILD_TYPE}" --print-errorlogs
                            '''
                        }
                    }
                }
            }
        }
    }

    post {
        always {
            junit allowEmptyResults: true, testResults: '**/meson-logs/testlog.junit.xml'
            archiveArtifacts artifacts: '**/meson-logs/**', allowEmptyArchive: true
        }

        failure {
            echo 'Build or tests failed. Check console log and meson-logs.'
        }
    }
}
