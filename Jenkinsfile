pipeline {
    agent any
    environment {
        PICO_SDK_PATH = "${WORKSPACE}/pico-sdk"
    }
    stages {
        stage('Prepare Repos') {
            steps {
                checkout poll: false, scm: scmGit(branches: [[name: '*/develop']], userRemoteConfigs: [[url: 'https://github.com/smittytone/Depot.git']])
                dir('pico-sdk') {
                    checkout poll: false, scm: scmGit(branches: [[name: '*/master']], extensions: [submodule(recursiveSubmodules: true, reference: '')], userRemoteConfigs: [[url: 'https://github.com/raspberrypi/pico-sdk.git']])
                }
            }
        }
        stage('Linux Apps Test Build') {
            steps {
                cmakeBuild buildDir: 'linux/build', cleanBuild: true, cmakeArgs: '-S', installation: 'InSearchPath', sourceDir: 'linux', steps: [[withCmake: true]]
            }
        }
        stage('Firmware Test Build') {
            steps {
                cmakeBuild buildDir: 'build', cleanBuild: true, cmakeArgs: '-S', installation: 'InSearchPath', steps: [[withCmake: true]]
            }
        }
    }
}
