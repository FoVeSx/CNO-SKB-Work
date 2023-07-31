import time
import os
import sys
import argparse
import json
import datetime
import zipfile
import subprocess

def mkzip(submission_filename, files):

    filenames = [os.path.abspath(x) for x in files]

    dirname = os.path.abspath(os.path.dirname(sys.argv[0]))
    
    if os.path.commonprefix([dirname] + filenames) != dirname:
        raise ValueError
        
    with zipfile.ZipFile(submission_filename + ".zip",'w') as z:

        for f in filenames:
            arcpath = os.path.relpath(f, dirname)

            while True:
                head, tail = os.path.split(arcpath)
                if head == "..":
                    raise ValueError
                if head == "":
                    break

            z.write(f, arcpath)
            

def main():
    subprocess.call(["docker", "image", "pull", "samthod3/cohereacademy:latest"])

    parser = argparse.ArgumentParser(description='Submits code to Autograder.')
    parser.add_argument('quiz', choices=['echo', 'transfer', 'gfclient', 'gfserver', 'gfclient_mt', 'gfserver_mt'])
    #parser.add_argument('--provider', choices=['gt', 'udacity'], default='gt')
    #parser.add_argument('--environment', choices=['local', 'development', 'staging', 'production'],
    #                    default='production')

    args = parser.parse_args()

    path_map = {'echo': 'echo',
                'transfer': 'transfer',
                'gfclient': 'gflib',
                'gfserver': 'gflib',
                'gfclient_mt': 'mtgf',
                'gfserver_mt': 'mtgf'}

    quiz_map = {'echo': 'pr1_echo_client_server',
                'transfer': 'pr1_transfer',
                'gfclient': 'pr1_gfclient',
                'gfserver': 'pr1_gfserver',
                'gfclient_mt': 'pr1_gfclient_mt',
                'gfserver_mt': 'pr1_gfserver_mt'}

    files_map = {'pr1_echo_client_server': ['echoclient.c', 'echoserver.c'],
                 'pr1_transfer': ['transferclient.c', 'transferserver.c'],
                 'pr1_gfclient': ['gfclient.c'],
                 'pr1_gfserver': ['gfserver.c'],
                 'pr1_gfclient_mt': ['gfclient_download.c'],
                 'pr1_gfserver_mt': ['gfserver_main.c', 'handler.c']}

    quiz = quiz_map[args.quiz]

    os.chdir(path_map[args.quiz])

    filenames=files_map[quiz]
    mkzip(args.quiz, filenames)

    
    srcZip = os.getcwd() + "/" + args.quiz + ".zip"
    dstZip = "/autograder/submission/" + args.quiz + ".zip"

    mountSubmission = srcZip + ":" + dstZip
    mountResults = os.getcwd() + ":/autograder/results"

    subprocess.call(["docker", "run", "-it", "-v", mountSubmission, "-v", mountResults, "samthod3/cohereacademy:latest", "/autograder/run_autograder"])
    
    timestamp = "{:-%Y-%m-%d-%H-%M-%S}".format(datetime.datetime.now())
    filename = "%s-result%s.json" % (args.quiz, timestamp)
    os.rename(os.getcwd() + "/results.json", os.getcwd() + "/" + filename)
    
    with open(os.getcwd() + "/" + filename, "r") as fd:
        result = json.load(fd)
    
    print(json.dumps(result, indent=4, separators=(',', ': ')))

    print (f"(Results saved in {os.path.join(path_map[args.quiz], filename)}")



if __name__ == '__main__':
    main()
