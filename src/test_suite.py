import os
import sys
import subprocess
import time
import json


tests = {
    'all_tests': [
        {'formula': '', 'test_files': []},
    ],
    'basic_tests': [
        {'test_name': 'true', 'formula': 'true', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'false', 'formula': 'false', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': '!true', 'formula': '!true', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': '!false', 'formula': '!false', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'true && true', 'formula': 'true && true', 'test_files': ["1_cycle_true.txt"]},
        {'test_name': 'true && false', 'formula': 'true && false', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': 'false && true', 'formula': 'false && true', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': 'false && false', 'formula': 'false && false', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': 'true || true', 'formula': 'true || true', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'true || false', 'formula': 'true || false', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'false || true', 'formula': 'false || true', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'false || false', 'formula': 'false || false', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': 'true -> true', 'formula': 'true -> true', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'true -> false', 'formula': 'true -> false', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': 'false -> true', 'formula': 'false -> true', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'false -> false', 'formula': 'false -> false', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'true <-> true', 'formula': 'true <-> true', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'true <-> false', 'formula': 'true <-> false', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': 'false <-> true', 'formula': 'false <-> true', 'test_files': ["1_cycle_false.txt",]},
        {'test_name': 'false <-> false', 'formula': 'false <-> false', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'ast_rotation', 'formula': '2.0*8.0+9.0-5.0*9.0+4.0-9.0-5.0+8.0-6.0 == -28.0', 'test_files': ["1_cycle_true.txt",]},
        {'test_name': 'ast_rotation', 'formula': '2.0*(6.0-4.0+5.0)*((((((((((((((((((((6.0))))))))))))))))))))-54.0+8.0+4.0-5.0-5.0-5.0-----8.0+4.0 == 23.0', 'test_files': ["1_cycle_true.txt",]},
    ],
    'single_ALWAYS': [
        {'test_name': 'single ALWAYS', 'formula': '[]x==1', 'test_files': ["1_cycle_true.txt", "1_cycle_false.txt", "2_cycle_true.txt", "2_cycle_false.txt",]},
        {'test_name': 'single ALWAYS', 'formula': '[]_{1}x==1', 'test_files': ["1_cycle_true.txt", "1_cycle_false.txt", "2_cycle_true.txt", "2_cycle_false.txt",]},
    ],
    'single_EVENTUALLY': [
        {'test_name': 'single EVENTUALLY', 'formula': '<>x==1', 'test_files': ["1_cycle_true.txt", "1_cycle_false.txt", "2_cycle_true.txt", "2_cycle_false.txt", "3_cycle_true.txt",]},
        {'test_name': 'single EVENTUALLY', 'formula': '<>_{x==1}1', 'test_files': ["1_cycle_true.txt", "1_cycle_false.txt", "2_cycle_true.txt", "2_cycle_false.txt", "3_cycle_true.txt",]},
    ],
    'single_NEXT': [
        {'test_name': 'single NEXT', 'formula': 'Ox==1', 'test_files': ["1_cycle_true.txt", "2_cycle_true.txt", "2_cycle_false.txt",]},
    ],
    'single_RELEASE': [
        {'formula': 'x<4Vy==3', 'test_files': ["1_cycle_false.txt","2_cycle_true_1.txt","2_cycle_true_2.txt","2_cycle_false_1.txt","2_cycle_false_2.txt","more_cycle_true_1.txt","more_cycle_true_2.txt","more_cycle_false_1.txt",]},
    ],
    'single_UNTIL': [
        {'test_name': 'single UNTIL', 'formula': 'x<4Ux==7', 'test_files': ["1_cycle_true.txt", "1_cycle_false_1.txt", "1_cycle_false_2.txt", "2_cycle_true_1.txt", "2_cycle_true_2.txt", "2_cycle_false_1.txt", "2_cycle_false_2.txt", "more_cycle_true_1.txt", "more_cycle_true_2.txt", "more_cycle_false_1.txt",]},
        {'test_name': 'single UNTIL', 'formula': 'x<4Ux==3', 'test_files': ["more_cycle_true_3.txt",]},
    ],
    'compound_ALWAYS': [
        {'formula': '[]Ox==1', 'test_files': ["ALWAYS_NEXT_true.txt", "ALWAYS_NEXT_false.txt",]},
        {'formula': '[]OOOx==1', 'test_files': ["ALWAYS_NEXT_NEXT_NEXT_true.txt", "ALWAYS_NEXT_NEXT_NEXT_false.txt",]},
        {'formula': '[]<>x==1', 'test_files': ["ALWAYS_EVENTUALLY_true_1.txt", "ALWAYS_EVENTUALLY_true_2.txt", "ALWAYS_EVENTUALLY_false_1.txt",]},
        {'formula': '[]<>OOx==1', 'test_files': ["ALWAYS_EVENTUALLY_NEXT_NEXT_true_1.txt", "ALWAYS_EVENTUALLY_NEXT_NEXT_true_2.txt", "ALWAYS_EVENTUALLY_NEXT_NEXT_false_1.txt", "ALWAYS_EVENTUALLY_NEXT_NEXT_false_2.txt",]},
    ],
    'compound_EVENTUALLY': [
        {'formula': '<>Ox==1', 'test_files': ["EVENTUALLY_NEXT_true.txt", "EVENTUALLY_NEXT_false.txt",]},
        {'formula': '<>OOOx==1', 'test_files': ["EVENTUALLY_NEXT_NEXT_NEXT_true.txt", "EVENTUALLY_NEXT_NEXT_NEXT_false.txt",]},
        {'formula': '<>[]x==1', 'test_files': ["EVENTUALLY_ALWAYS_true_1.txt", "EVENTUALLY_ALWAYS_true_2.txt", "EVENTUALLY_ALWAYS_false_1.txt", "EVENTUALLY_ALWAYS_false_2.txt",]},
        {'formula': '<>[]OOx==1', 'test_files': ["EVENTUALLY_ALWAYS_NEXT_NEXT_true_1.txt", "EVENTUALLY_ALWAYS_NEXT_NEXT_true_2.txt", "EVENTUALLY_ALWAYS_NEXT_NEXT_false_1.txt", "EVENTUALLY_ALWAYS_NEXT_NEXT_false_2.txt",]},
    ],
    'compound_logic_ltl': [
        {'formula': 'x==1 -> <>y>2', 'test_files': ["pCONSECUENCE_EVENTUALLYq_true_1.txt", "pCONSECUENCE_EVENTUALLYq_true_2.txt", "pCONSECUENCE_EVENTUALLYq_true_3.txt", "pCONSECUENCE_EVENTUALLYq_false.txt",]},
        {'formula': 'x==1 -> y<4 U y<x', 'test_files': ["pCONSECUENCE_q_UNTIL_r_true.txt", "pCONSECUENCE_q_UNTIL_r_false.txt",]},
        {'formula': '<>x==1 <-> <>y==2', 'test_files': ["EVENTUALLY_p_BICONDITIONAL_EVENTUALLY_q_true_1.txt", "EVENTUALLY_p_BICONDITIONAL_EVENTUALLY_q_true_2.txt", "EVENTUALLY_p_BICONDITIONAL_EVENTUALLY_q_false_1.txt", "EVENTUALLY_p_BICONDITIONAL_EVENTUALLY_q_false_2.txt",]},
        {'formula': 'x==1->Ox==5', 'test_files': ["p_CONSECUENCE_NEXT_q_true_1.txt", "p_CONSECUENCE_NEXT_q_true_2.txt", "p_CONSECUENCE_NEXT_q_false_1.txt",]},
        {'formula': '[](x==1 -> Ox==5)', 'test_files': ["ALLWAYS_p_CONSECUENCE_NEXT_q_true_1.txt", "ALLWAYS_p_CONSECUENCE_NEXT_q_true_2.txt", "ALLWAYS_p_CONSECUENCE_NEXT_q_false_1.txt", "ALLWAYS_p_CONSECUENCE_NEXT_q_false_2.txt", "ALLWAYS_p_CONSECUENCE_NEXT_q_false_3.txt",]},
        {'formula': '[](x==1 -> <>x==5)', 'test_files': ["ALLWAYS_p_CONSECUENCE_EVENTUALLY_q_true_1.txt", "ALLWAYS_p_CONSECUENCE_EVENTUALLY_q_true_2.txt", "ALLWAYS_p_CONSECUENCE_EVENTUALLY_q_true_3.txt", "ALLWAYS_p_CONSECUENCE_EVENTUALLY_q_false_2.txt",]},
    ],
    'compound_NEXT': [
        {'formula': 'OOx==1', 'test_files': ["NEXT_NEXT_true.txt", "NEXT_NEXT_false.txt",]},
        {'formula': 'OOOx==1', 'test_files': ["NEXT_NEXT_NEXT_true.txt","NEXT_NEXT_NEXT_false.txt",]},
        {'formula': 'O<>x==1', 'test_files': ["NEXT_EVENTUALLY_true.txt","NEXT_EVENTUALLY_false.txt",]},
        {'formula': 'OOO<>x==1', 'test_files': ["NEXT_NEXT_NEXT_EVENTUALLY_true.txt","NEXT_NEXT_NEXT_EVENTUALLY_false.txt",]},
        {'formula': 'OO<>[]x==1', 'test_files': ["NEXT_NEXT_EVENTUALLY_ALWAYS_true_1.txt", "NEXT_NEXT_EVENTUALLY_ALWAYS_true_2.txt", "NEXT_NEXT_EVENTUALLY_ALWAYS_false_1.txt", "NEXT_NEXT_EVENTUALLY_ALWAYS_false_2.txt",]},
        {'formula': 'O[]x==1', 'test_files': ["NEXT_ALWAYS_true.txt", "NEXT_ALWAYS_false.txt",]},
        {'formula': 'OOO[]x==1', 'test_files': ["NEXT_NEXT_NEXT_ALWAYS_true.txt", "NEXT_NEXT_NEXT_ALWAYS_false.txt",]},
        {'formula': 'OO[]<>x==1', 'test_files': ["NEXT_NEXT_ALWAYS_EVENTUALLY_true_1.txt", "NEXT_NEXT_ALWAYS_EVENTUALLY_true_2.txt", "NEXT_NEXT_ALWAYS_EVENTUALLY_false_1.txt", "NEXT_NEXT_ALWAYS_EVENTUALLY_false_2.txt",]},
    ],
    'interval_ALWAYS': [
        {'formula': '[]_{1}x==2', 'test_files': ["interval_always_true_1.txt", "interval_always_true_2.txt", "interval_always_true_3.txt", "interval_always_true_4.txt", "interval_always_true_5.txt", "interval_always_false_1.txt", "interval_always_false_2.txt", ]},
        {'formula': '[]_{x==2}y==3', 'test_files': ["interval_always_2_true_1.txt", "interval_always_2_true_2.txt", "interval_always_2_true_3.txt", "interval_always_2_true_4.txt", "interval_always_2_true_5.txt", "interval_always_2_true_6.txt", "interval_always_2_false_1.txt", "interval_always_2_false_2.txt", "interval_always_2_false_3.txt", ]},
        {'formula': '[]_{x>=1, x>=9}y==3', 'test_files': ["interval_always_3_true_1.txt", "interval_always_3_true_2.txt", "interval_always_3_true_3.txt", "interval_always_3_false_1.txt", "interval_always_3_false_2.txt", "interval_always_3_false_3.txt", ]},
    ],
    'interval_EVENTUALLY': [
        {'formula': '<>_{x==2}1', 'test_files': ["interval_eventually_true_1.txt", "interval_eventually_true_2.txt", "interval_eventually_true_3.txt", "interval_eventually_true_4.txt", "interval_eventually_true_5.txt", "interval_eventually_false_1.txt", "interval_eventually_false_2.txt", ]},
        {'formula': '<>_{x==2}y==1', 'test_files': ["interval_eventually_2_true_1.txt", "interval_eventually_2_true_2.txt", "interval_eventually_2_true_3.txt", "interval_eventually_2_true_4.txt", "interval_eventually_2_true_5.txt", "interval_eventually_2_false_1.txt", "interval_eventually_2_false_2.txt", ]},
        {'formula': '[]_{x>=1, x >= 9}<>_{y==1}1', 'test_files': ["interval_eventually_3_true_1.txt", "interval_eventually_3_true_2.txt", "interval_eventually_3_true_3.txt", "interval_eventually_3_true_4.txt", "interval_eventually_3_true_5.txt", "interval_eventually_3_false_1.txt", "interval_eventually_3_false_2.txt", ]},
    ],
    'interval_UNTIL': [
        {'formula': "y==1 U_{x>=1, x>10} y==5", 'test_files': ["interval_until_true_1.txt", "interval_until_true_2.txt", "interval_until_true_3.txt", "interval_until_false_1.txt", "interval_until_false_2.txt", ]},
        {'formula': "y==1 U_{x==1} y==5", 'test_files': ["interval_until_2_true_1.txt", "interval_until_2_true_2.txt", "interval_until_2_false_1.txt", "interval_until_2_false_2.txt", ]},
    ],
    'interval_RELEASE': [
    ],
    'interval_COMPOUND': [
        {'formula': "[]_{status=='stt', status=='stp'}<>_{picture=='fp'}true", 'test_files': ["test_paper_true_1.txt", "test_paper_true_2.txt", "test_paper_true_3.txt", "test_paper_false_1.txt", "test_paper_false_2.txt", ]},
        {'formula': "[]_{command == 'stt', command == 'stp'}([]_{resolution == 'high', resolution == 'low'}rx_rate >= 5 && []_{resolution == 'low', resolution == 'high'}rx_rate <= 1)", 'test_files': ["test_paper_2_true_1.txt", "test_paper_2_true_2.txt", "test_paper_2_false_1.txt", "test_paper_2_false_2.txt", ]},
        {'formula': "[]_{status=='sleep', status=='awake'}[]_{true}glucose>=70", 'test_files': ["test_paper_3_true_1.csv", "test_paper_3_true_2.csv", "test_paper_3_true_3.csv", "test_paper_3_true_4.csv", "test_paper_3_false_1.csv", ]},
    ],
    'other_tests': [
        {'formula': "[]date < 2021-02-02T18:50:00.000", 'test_files': ["test_date.csv", ]},
        {'formula': "[]_{1}date < 2021-02-02T18:50:00.000", 'test_files': ["test_date.csv", ]},
        {'formula': "[]_{x>=2, x>3}avg(y)>2", 'test_files': ["test_avg.csv", ]},
    ],
}

equivalences = [
    ('<>x==1', '1Ux==1'),
    ('[]x==1', '0Vx==1'),
    ('[]x==1', '!<>!x==1'),
    ('[]Ox==1', 'O[]x==1'),
    ('<>Ox==1', 'O<>x==1'),
]


if __name__ == '__main__':
    print('--RUNNING TEST SUITE--\n')

    total_tests = 0
    success_tests = 0
    start = time.time()
    time_per_test = []

    for root, test_list in tests.items():
        print(f'Running {root}:')
        
        for test in test_list:
            formula = test['formula']

            for file in test['test_files']:
                test_file_path = os.path.join('..', 'test', root, file)

                command = f'ltl_interpreter.exe -f "{formula}" -i "{test_file_path}"'

                start_per_test = time.time()
                out = subprocess.run(
                    command,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    shell=True,
                )
                end_per_test = time.time()
                time_per_test.append(end_per_test - start_per_test)

                if out.stderr:
                    print(command)
                    print(f'\033[91mERROR 0\033[0m: \"{out.stderr.decode("utf-8")}\"')
                    exit(-1)

                test_should_be_true = 'true' in file
                result = out.stdout.decode('utf-8')

                if not result:
                    print(command)
                    print(f'    \033[91mFAILED\033[0m: \033[93m{formula}\033[0m - {file}')
                    print(f'      \033[91mempty result: "{result}"\033[0m')
                    continue
                
                result_processed = result.strip().split(":")
                # print(f'        {test_file_path}')
                # print(f'        result: {result}')
                test_ended_true = 'true' in result_processed[1]
                
                total_tests += 1

                if test_should_be_true == test_ended_true:
                    success_tests += 1
                else:
                    print(command)
                    print(f'    \033[91mFAILED\033[0m: \033[93m{formula}\033[0m - {file}')
                    print(f'              \033[91mresult: {result.strip()}\033[0m')

    print(f'Running equivalences:')

    for formulas in equivalences:
        print(f'  -{formulas[0]} == {formulas[1]}')

        for file in os.listdir(os.path.join('..', 'test', 'all_tests')):
            test_file_path = os.path.join('..', 'test', 'all_tests', file)

            command = f'ltl_interpreter.exe -f "{formulas[0]}" -i {test_file_path}'

            start_per_test = time.time()
            out = subprocess.run(
                command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                shell=True,
            )
            end_per_test = time.time()
            time_per_test.append(end_per_test - start_per_test)

            if out.stderr:
                print(f'\033[91mERROR 1\033[0m: \"{out.stderr.decode("utf-8")}\"')
                exit(-1)

            test_result_1 = 'true' in out.stdout.decode('utf-8')

            command = f'ltl_interpreter.exe -f "{formulas[1]}" -i {test_file_path}'

            start_per_test = time.time()
            out = subprocess.run(
                command,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                shell=True,
            )
            end_per_test = time.time()
            time_per_test.append(end_per_test - start_per_test)

            if out.stderr:
                print(f'\033[91mERROR 2\033[0m: \"{out.stderr.decode("utf-8")}\"')
                exit(-1)

            test_result_2 = 'true' in out.stdout.decode('utf-8')
            
            total_tests += 1

            if test_result_1 == test_result_2:
                success_tests += 1
            else:
                print(f'\tFAILED: {formulas} - {file}')
                
                if out.stderr:
                    print(f'ERROR in equivalences: \"{out.stderr.decode("utf-8")}\"')

    print("\nTests finished");
    print("---------------");
    print(f'Passed:\t{success_tests}');
    print(f'Total:\t{total_tests}');
    end = time.time()
    print(f'Time spent: {"{:.2f}".format(end - start)} s')
    print(f'Average time per test: {"{:.4f}".format(sum(time_per_test) * 1000 / len(time_per_test))} ms')
