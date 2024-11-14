pipeline {
    agent any
    stages {
        stage('Git Prep') {
            steps {
                checkout poll: false, scm: scmGit(branches: [[name: '*/develop']], userRemoteConfigs: [[url: 'https://github.com/smittytone/Depot.git']])
                sh 'ls linux'
                cmakeBuild buildDir: 'linux/build', cleanBuild: true, cmakeArgs: '-S', installation: 'InSearchPath', sourceDir: 'linux', steps: [[withCmake: true]]
            }
        }
        stage('Firmware Test Build') {
            steps {
                cmakeBuild buildDir: 'build', cleanBuild: true, installation: 'InSearchPath', steps: [[withCmake: true]]
            }
        }
    }
}
