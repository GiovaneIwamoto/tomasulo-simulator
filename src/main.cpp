#include <systemc.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/menubar.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/filebox.hpp>
#include "top.hpp"
#include "gui.hpp"

using std::fstream;
using std::map;
using std::string;
using std::vector;

int sc_main(int argc, char *argv[])
{
    using namespace nana;
    vector<string> instruction_queue;
    string bench_name = "";
    int nadd, nmul, nls, n_bits, bpb_size, cpu_freq, m_size;
    m_size = 2;
    nadd = 3;
    nmul = nls = 2;
    n_bits = 2;
    bpb_size = 4;
    cpu_freq = 500; // definido em Mhz - 500Mhz default
    std::vector<int> sizes;
    bool spec = false;
    int mode = 0;
    bool fila = false;
    ifstream inFile;
    form fm(API::make_center(1024, 700));
    place plc(fm);
    place upper(fm);
    place lower(fm);
    listbox table(fm);
    listbox reg(fm);
    listbox instruct(fm);
    listbox rob(fm);
    menubar mnbar(fm);
    button start(fm);
    button run_all(fm);
    button clock_control(fm);
    button exit(fm);
    group clock_group(fm);
    label clock_count(clock_group);
    fm.caption("TFSim");
    clock_group.caption("Ciclo");
    clock_group.div("count");
    grid memory(fm, rectangle(), 10, 50);
    // Tempo de latencia de uma instrucao
    // Novas instrucoes devem ser inseridas manualmente aqui
    map<string, int> instruct_time{{"DADD", 4}, {"DADDI", 4}, {"DSUB", 6}, {"DSUBI", 6}, {"DMUL", 10}, {"DDIV", 16}, {"MEM", 2}, {"SLT", 1}, {"SGT", 1}};
    // Responsavel pelos modos de execução
    top top1("top");
    start.caption("Start");
    clock_control.caption("Next cycle");
    run_all.caption("Run all");
    exit.caption("Exit");
    plc["rst"] << table;
    plc["btns"] << start << clock_control << run_all << exit;
    plc["memor"] << memory;
    plc["regs"] << reg;
    plc["rob"] << rob;
    plc["instr"] << instruct;
    plc["clk_c"] << clock_group;
    clock_group["count"] << clock_count;
    clock_group.collocate();

    spec = false;
    // set_spec eh so visual
    set_spec(plc, spec);
    plc.collocate();

    mnbar.push_back("Opções");
    menu &op = mnbar.at(0);
    // menu::item_proxy spec_ip =
    op.append("Especulação");
    auto spec_sub = op.create_sub_menu(0);
    // Modo com 1 preditor para todos os branchs
    spec_sub->append("1 Preditor", [&](menu::item_proxy &ip)
                     {
        if(ip.checked()){
            spec = true;
            mode = 1;
            spec_sub->checked(1, false);
            spec_sub->checked(2, false);
        }
        else{
            spec = false;
            mode = 0;
        }

        set_spec(plc,spec); });
    spec_sub->append("2 Preditor M,N", [&](menu::item_proxy &ip)
                     {
        if (ip.checked())
        {
            spec = true;
            mode = 3;
            spec_sub->checked(0, false);
            spec_sub->checked(2, false);
        }
        else
        {
            spec = false;
            mode = 0;
        }
                set_spec(plc,spec); });

    // Modo com o bpb
    spec_sub->append("Branch Prediction Buffer", [&](menu::item_proxy &ip)
                     {
        if (ip.checked())
        {
            spec = true;
            mode = 2;
            spec_sub->checked(0, false);
            spec_sub->checked(1, false);

        }
        else
        {
            spec = false;
            mode = 0;
        }

        set_spec(plc, spec); });
    spec_sub->check_style(0, menu::checks::highlight);
    spec_sub->check_style(1, menu::checks::highlight);
    spec_sub->check_style(2, menu::checks::highlight);

    op.append("Modificar valores...");
    // novo submenu para escolha do tamanho do bpb e do preditor
    auto sub = op.create_sub_menu(1);
    sub->append("Tamanho do BPB e Preditor", [&](menu::item_proxy &ip)
                {
        inputbox ibox(fm, "", "Definir tamanhos");
        inputbox::integer size("BPB", bpb_size, 2, 10, 2);
        inputbox::integer bits("N_BITS", n_bits, 1, 3, 1);
        inputbox::integer m_size_input("M_SIZE", m_size, 1, 8, 1);

        if (ibox.show_modal(size, bits, m_size_input))
        {
            bpb_size = size.value();
            n_bits = bits.value();
            m_size = m_size_input.value();
        } });
    sub->append("Número de Estações de Reserva", [&](menu::item_proxy ip)
                {
        inputbox ibox(fm, "", "Quantidade de Estações de Reserva");
        inputbox::integer add("ADD/SUB", nadd, 1, 10, 1);
        inputbox::integer mul("MUL/DIV", nmul, 1, 10, 1);
        inputbox::integer sl("LOAD/STORE", nls, 1, 10, 1);
        if (ibox.show_modal(add, mul, sl))
        {
            nadd = add.value();
            nmul = mul.value();
            nls = sl.value();
        } });
    // Menu de ajuste dos tempos de latencia na interface
    // Novas instrucoes devem ser adcionadas manualmente aqui
    sub->append("Tempos de latência", [&](menu::item_proxy &ip)
                {
        inputbox ibox(fm, "", "Tempos de latência para instruções");
        inputbox::text dadd_t("DADD", std::to_string(instruct_time["DADD"]));
        inputbox::text daddi_t("DADDI", std::to_string(instruct_time["DADDI"]));
        inputbox::text dsub_t("DSUB", std::to_string(instruct_time["DSUB"]));
        inputbox::text dsubi_t("DSUBI", std::to_string(instruct_time["DSUBI"]));
        inputbox::text dmul_t("DMUL", std::to_string(instruct_time["DMUL"]));
        inputbox::text ddiv_t("DDIV", std::to_string(instruct_time["DDIV"]));
        inputbox::text mem_t("Load/Store", std::to_string(instruct_time["MEM"]));
        if (ibox.show_modal(dadd_t, daddi_t, dsub_t, dsubi_t, dmul_t, ddiv_t, mem_t))
        {
            instruct_time["DADD"] = std::stoi(dadd_t.value());
            instruct_time["DADDI"] = std::stoi(daddi_t.value());
            instruct_time["DSUB"] = std::stoi(dsub_t.value());
            instruct_time["DSUBI"] = std::stoi(dsubi_t.value());
            instruct_time["DMUL"] = std::stoi(dmul_t.value());
            instruct_time["DDIV"] = std::stoi(ddiv_t.value());
            instruct_time["MEM"] = std::stoi(mem_t.value());
        } });

    sub->append("Frequência CPU", [&](menu::item_proxy &ip)
                {
        inputbox ibox(fm, "Em Mhz", "Definir frequência da CPU");
        inputbox::text freq("Frequência", std::to_string(cpu_freq));
        if (ibox.show_modal(freq))
        {
            cpu_freq = std::stoi(freq.value());
        } });

    sub->append("Fila de instruções", [&](menu::item_proxy &ip)
                {
        filebox fb(0, true);
        inputbox ibox(fm, "Localização do arquivo com a lista de instruções:");
        inputbox::path caminho("", fb);
        if (ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if (!add_instructions(inFile, instruction_queue, instruct))
                show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
            else
                fila = true;
        } });
    sub->append("Valores de registradores inteiros", [&](menu::item_proxy &ip)
                {
        filebox fb(0, true);
        inputbox ibox(fm, "Localização do arquivo de valores de registradores inteiros:");
        inputbox::path caminho("", fb);
        if (ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if (!inFile.is_open())
                show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
            else
            {
                auto reg_gui = reg.at(0);
                int value, i = 0;
                while (inFile >> value && i < 32)
                {
                    reg_gui.at(i).text(1, std::to_string(value));
                    i++;
                }
                for (; i < 32; i++)
                    reg_gui.at(i).text(1, "0");
                inFile.close();
            }
        } });
    sub->append("Valores de registradores PF", [&](menu::item_proxy &ip)
                {
        filebox fb(0, true);
        inputbox ibox(fm, "Localização do arquivo de valores de registradores PF:");
        inputbox::path caminho("", fb);
        if (ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if (!inFile.is_open())
                show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
            else
            {
                auto reg_gui = reg.at(0);
                int i = 0;
                float value;
                while (inFile >> value && i < 32)
                {
                    reg_gui.at(i).text(4, std::to_string(value));
                    i++;
                }
                for (; i < 32; i++)
                    reg_gui.at(i).text(4, "0");
                inFile.close();
            }
        } });
    sub->append("Valores de memória", [&](menu::item_proxy &ip)
                {
        filebox fb(0, true);
        inputbox ibox(fm, "Localização do arquivo de valores de memória:");
        inputbox::path caminho("", fb);
        if (ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if (!inFile.is_open())
                show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
            else
            {
                int i = 0;
                int value;
                while (inFile >> value && i < 500)
                {
                    memory.Set(i, std::to_string(value));
                    i++;
                }
                for (; i < 500; i++)
                {
                    memory.Set(i, "0");
                }
                inFile.close();
            }
        } });
    op.append("Verificar conteúdo...");
    auto new_sub = op.create_sub_menu(2);
    new_sub->append("Valores de registradores", [&](menu::item_proxy &ip)
                    {
        filebox fb(0, true);
        inputbox ibox(fm, "Localização do arquivo de valores de registradores:");
        inputbox::path caminho("", fb);
        if (ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if (!inFile.is_open())
                show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
            else
            {
                string str;
                int reg_pos, i;
                bool is_float, ok;
                ok = true;
                while (inFile >> str)
                {
                    if (str[0] == '$')
                    {
                        if (str[1] == 'f')
                        {
                            i = 2;
                            is_float = true;
                        }
                        else
                        {
                            i = 1;
                            is_float = false;
                        }
                        reg_pos = std::stoi(str.substr(i, str.size() - i));

                        auto reg_gui = reg.at(0);
                        string value;
                        inFile >> value;
                        if (!is_float)
                        {
                            if (reg_gui.at(reg_pos).text(1) != value)
                            {
                                ok = false;
                                break;
                            }
                        }
                        else
                        {
                            if (std::stof(reg_gui.at(reg_pos).text(4)) != std::stof(value))
                            {
                                ok = false;
                                break;
                            }
                        }
                    }
                }
                inFile.close();
                msgbox msg("Verificação de registradores");
                if (ok)
                {
                    msg << "Registradores especificados apresentam valores idênticos!";
                    msg.icon(msgbox::icon_information);
                }
                else
                {
                    msg << "Registradores especificados apresentam valores distintos!";
                    msg.icon(msgbox::icon_error);
                }
                msg.show();
            }
        } });
    new_sub->append("Valores de memória", [&](menu::item_proxy &ip)
                    {
        filebox fb(0, true);
        inputbox ibox(fm, "Localização do arquivo de valores de memória:");
        inputbox::path caminho("", fb);
        if (ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if (!inFile.is_open())
                show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
            else
            {
                string value;
                bool ok;
                ok = true;
                for (int i = 0; i < 500; i++)
                {
                    inFile >> value;
                    if (std::stoi(memory.Get(i)) != (int)std::stol(value, nullptr, 16))
                    {
                        ok = false;
                        break;
                    }
                }
                inFile.close();
                msgbox msg("Verificação de memória");
                if (ok)
                {
                    msg << "Endereços de memória especificados apresentam valores idênticos!";
                    msg.icon(msgbox::icon_information);
                }
                else
                {
                    msg << "Endereços de memória especificados apresentam valores distintos!";
                    msg.icon(msgbox::icon_error);
                }
                msg.show();
            }
        } });
    op.append("Benchmarks");
    auto bench_sub = op.create_sub_menu(3);
    bench_sub->append("Fibonacci", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/fibonacci/fibonacci.txt";
        bench_name = "fibonacci";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
            fila = true; });
    bench_sub->append("Stall por Divisão", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/division_stall.txt";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
            fila = true; });
    bench_sub->append("Stress de Memória (Stores)", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/store_stress/store_stress.txt";
        bench_name = "store_stress";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
            fila = true; });
    bench_sub->append("Stall por hazard estrutural (Adds)", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/res_stations_stall.txt";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
            fila = true; });
    bench_sub->append("Busca Linear", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/linear_search/linear_search.txt";
        bench_name = "linear_search";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo");
        else
            fila = true;

        path = "in/benchmarks/linear_search/memory.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            int i = 0;
            int value;
            while (inFile >> value && i < 500)
            {
                memory.Set(i, std::to_string(value));
                i++;
            }
            for (; i < 500; i++)
            {
                memory.Set(i, "0");
            }
            inFile.close();
        }

        path = "in/benchmarks/linear_search/regi_i.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            auto reg_gui = reg.at(0);
            int value, i = 0;
            while (inFile >> value && i < 32)
            {
                reg_gui.at(i).text(1, std::to_string(value));
                i++;
            }
            for (; i < 32; i++)
                reg_gui.at(i).text(1, "0");
            inFile.close();
        } });

    bench_sub->append("Busca Binária", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/binary_search/binary_search.txt";
        bench_name = "binary_search";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo");
        else
            fila = true;

        path = "in/benchmarks/binary_search/memory.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            int i = 0;
            int value;
            while (inFile >> value && i < 500)
            {
                memory.Set(i, std::to_string(value));
                i++;
            }
            for (; i < 500; i++)
            {
                memory.Set(i, "0");
            }
            inFile.close();
        }

        path = "in/benchmarks/binary_search/regs.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            auto reg_gui = reg.at(0);
            int value, i = 0;
            while (inFile >> value && i < 32)
            {
                reg_gui.at(i).text(1, std::to_string(value));
                i++;
            }
            for (; i < 32; i++)
                reg_gui.at(i).text(1, "0");
            inFile.close();
        } });

    bench_sub->append("Matriz Search", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/matriz_search/matriz_search.txt";
        bench_name = "matriz_search";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo");
        else
            fila = true;

        path = "in/benchmarks/matriz_search/memory.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            int i = 0;
            int value;
            while (inFile >> value && i < 500)
            {
                memory.Set(i, std::to_string(value));
                i++;
            }
            for (; i < 500; i++)
            {
                memory.Set(i, "0");
            }
            inFile.close();
        }

        path = "in/benchmarks/matriz_search/regs.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            auto reg_gui = reg.at(0);
            int value, i = 0;
            while (inFile >> value && i < 32)
            {
                reg_gui.at(i).text(1, std::to_string(value));
                i++;
            }
            for (; i < 32; i++)
                reg_gui.at(i).text(1, "0");
            inFile.close();
        } });

    bench_sub->append("Bubble Sort", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/bubble_sort/bubble_sort.txt";
        bench_name = "bubble_sort";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo");
        else
            fila = true;

        path = "in/benchmarks/bubble_sort/memory.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            int i = 0;
            int value;
            while (inFile >> value && i < 500)
            {
                memory.Set(i, std::to_string(value));
                i++;
            }
            for (; i < 500; i++)
            {
                memory.Set(i, "0");
            }
            inFile.close();
        }

        path = "in/benchmarks/bubble_sort/regs.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            auto reg_gui = reg.at(0);
            int value, i = 0;
            while (inFile >> value && i < 32)
            {
                reg_gui.at(i).text(1, std::to_string(value));
                i++;
            }
            for (; i < 32; i++)
                reg_gui.at(i).text(1, "0");
            inFile.close();
        } });

    bench_sub->append("Insertion Sort", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/insertion_sort/insertion_sort.txt";
        bench_name = "insertion_sort";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo");
        else
            fila = true;

        path = "in/benchmarks/insertion_sort/memory.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            int i = 0;
            int value;
            while (inFile >> value && i < 500)
            {
                memory.Set(i, std::to_string(value));
                i++;
            }
            for (; i < 500; i++)
            {
                memory.Set(i, "0");
            }
            inFile.close();
        }

        path = "in/benchmarks/insertion_sort/regs.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            auto reg_gui = reg.at(0);
            int value, i = 0;
            while (inFile >> value && i < 32)
            {
                reg_gui.at(i).text(1, std::to_string(value));
                i++;
            }
            for (; i < 32; i++)
                reg_gui.at(i).text(1, "0");
            inFile.close();
        } });

    bench_sub->append("Tick Tack", [&](menu::item_proxy &ip)
                      {
        string path = "in/benchmarks/tick_tack/tick_tack.txt";
        bench_name = "tick_tack";
        inFile.open(path);
        if (!add_instructions(inFile, instruction_queue, instruct))
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo");
        else
            fila = true;

        path = "in/benchmarks/tick_tack/regs.txt";
        inFile.open(path);
        if (!inFile.is_open())
            show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
        else
        {
            auto reg_gui = reg.at(0);
            int value, i = 0;
            while (inFile >> value && i < 32)
            {
                reg_gui.at(i).text(1, std::to_string(value));
                i++;
            }
            for (; i < 32; i++)
                reg_gui.at(i).text(1, "0");
            inFile.close();
        } });

    vector<string> columns = {"#", "Name", "Busy", "Op", "Vj", "Vk", "Qj", "Qk", "A"};
    for (unsigned int i = 0; i < columns.size(); i++)
    {
        table.append_header(columns[i].c_str());
        if (i && i != 3)
            table.column_at(i).width(45);
        else if (i == 3)
            table.column_at(i).width(60);
        else
            table.column_at(i).width(30);
    }
    columns = {"", "Value", "Qi"};
    sizes = {30, 75, 40};
    for (unsigned int k = 0; k < 2; k++)
        for (unsigned int i = 0; i < columns.size(); i++)
        {
            reg.append_header(columns[i]);
            reg.column_at(k * columns.size() + i).width(sizes[i]);
        }

    auto reg_gui = reg.at(0);
    for (int i = 0; i < 32; i++)
    {
        string index = std::to_string(i);
        reg_gui.append("R" + index);
        reg_gui.at(i).text(3, "F" + index);
    }

    columns = {"Instruction", "Issue", "Execute", "Write Result"};
    sizes = {140, 60, 70, 95};
    for (unsigned int i = 0; i < columns.size(); i++)
    {
        instruct.append_header(columns[i]);
        instruct.column_at(i).width(sizes[i]);
    }
    columns = {"Entry", "Busy", "Instruction", "State", "Destination", "Value"};
    sizes = {45, 45, 120, 120, 90, 60};
    for (unsigned int i = 0; i < columns.size(); i++)
    {
        rob.append_header(columns[i]);
        rob.column_at(i).width(sizes[i]);
    }

    srand(static_cast<unsigned>(time(0)));
    for (int i = 0; i < 32; i++)
    {
        if (i)
            reg_gui.at(i).text(1, std::to_string(rand() % 100));
        else
            reg_gui.at(i).text(1, std::to_string(0));
        reg_gui.at(i).text(2, "0");
        reg_gui.at(i).text(4, std::to_string(static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 100.0)));
        reg_gui.at(i).text(5, "0");
    }
    for (int i = 0; i < 500; i++)
        memory.Push(std::to_string(rand() % 100));
    for (int k = 1; k < argc; k += 2)
    {
        int i;
        if (strlen(argv[k]) > 2)
            show_message("Opção inválida", string("Opção \"") + string(argv[k]) + string("\" inválida"));
        else
        {
            char c = argv[k][1];
            switch (c)
            {
            case 'q':
                inFile.open(argv[k + 1]);
                if (!add_instructions(inFile, instruction_queue, instruct))
                    show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
                else
                    fila = true;
                break;
            case 'i':
                inFile.open(argv[k + 1]);
                int value;
                i = 0;
                if (!inFile.is_open())
                    show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
                else
                {
                    while (inFile >> value && i < 32)
                    {
                        reg_gui.at(i).text(1, std::to_string(value));
                        i++;
                    }
                    for (; i < 32; i++)
                        reg_gui.at(i).text(1, "0");
                    inFile.close();
                }
                break;
            case 'f':
                float value_fp;
                i = 0;
                inFile.open(argv[k + 1]);
                if (!inFile.is_open())
                    show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
                else
                {
                    while (inFile >> value_fp && i < 32)
                    {
                        reg_gui.at(i).text(4, std::to_string(value_fp));
                        i++;
                    }
                    for (; i < 32; i++)
                        reg_gui.at(i).text(4, "0");
                    inFile.close();
                }
                break;
            case 'm':
                inFile.open(argv[k + 1]);
                if (!inFile.is_open())
                    show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
                else
                {
                    int value;
                    i = 0;
                    while (inFile >> value && i < 500)
                    {
                        memory.Set(i, std::to_string(value));
                        i++;
                    }
                    for (; i < 500; i++)
                    {
                        memory.Set(i, "0");
                    }
                    inFile.close();
                }
                break;
            case 'r':
                inFile.open(argv[k + 1]);
                if (!inFile.is_open())
                    show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
                else
                {
                    int value;
                    if (inFile >> value && value <= 10 && value > 0)
                        nadd = value;
                    if (inFile >> value && value <= 10 && value > 0)
                        nmul = value;
                    if (inFile >> value && value <= 10 && value > 0)
                        nls = value;
                    inFile.close();
                }
                break;
            case 's':
                spec = true;
                set_spec(plc, spec);
                // spec_ip.checked(true);
                k--;
                break;
            case 'l':
                inFile.open(argv[k + 1]);
                if (!inFile.is_open())
                    show_message("Arquivo inválido", "Não foi possível abrir o arquivo!");
                else
                {
                    string inst;
                    int value;
                    while (inFile >> inst)
                    {
                        if (inFile >> value && instruct_time.count(inst))
                            instruct_time[inst] = value;
                    }
                }
                inFile.close();
                break;
            default:
                show_message("Opção inválida", string("Opção \"") + string(argv[k]) + string("\" inválida"));
                break;
            }
        }
    }

    clock_control.enabled(false);
    run_all.enabled(false);

    start.events().click([&]
                         {
        if (fila)
        {
            start.enabled(false);
            clock_control.enabled(true);
            run_all.enabled(true);
            // Desativa os menus apos inicio da execucao
            op.enabled(0, false);
            op.enabled(1, false);
            op.enabled(3, false);
            for (int i = 0; i < 2; i++)
                spec_sub->enabled(i, false);
            for (int i = 0; i < 8; i++)
                sub->enabled(i, false);
            for (int i = 0; i < 10; i++)
                bench_sub->enabled(i, false);
            if (spec)
            {
                // Flag mode setada pela escolha no menu
                if (mode == 1)
                    top1.rob_mode(n_bits, nadd, nmul, nls, instruct_time, instruction_queue, table, memory, reg, instruct, clock_count, rob);
                else if(mode == 3)
                    top1.rob_mode_mn(n_bits,nadd,nmul,nls,instruct_time,instruction_queue,table,memory,reg,instruct,clock_count,rob, m_size);
                else if (mode == 2)
                    top1.rob_mode_bpb(n_bits, bpb_size, nadd, nmul, nls, instruct_time, instruction_queue, table, memory, reg, instruct, clock_count, rob);
            }
            else
                top1.simple_mode(nadd, nmul, nls, instruct_time, instruction_queue, table, memory, reg, instruct, clock_count);
            sc_start();
        }
        else
            show_message("Fila de instruções vazia", "A fila de instruções está vazia. Insira um conjunto de instruções para iniciar."); });

    clock_control.events().click([&]
                                 {
        if (top1.get_queue().queue_is_empty() && top1.get_rob().rob_is_empty())
            return;
        if (sc_is_running())
            sc_start(); });

    run_all.events().click([&]
                           {
        // enquanto queue e rob nao estao vazios, roda ate o fim
        while (!(top1.get_queue().queue_is_empty() && top1.get_rob().rob_is_empty()))
        {
            if (sc_is_running())
                sc_start();
        }

        top1.metrics(cpu_freq, mode, bench_name, n_bits); });

    exit.events().click([]
                        {
        sc_stop();
        API::exit(); });
    fm.show();
    exec();
    return 0;
}
