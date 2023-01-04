#include "bnkextr.hpp"

int Swap32(const uint32_t dword)
{
#ifdef __GNUC__
	return __builtin_bswap32(dword);
#elif _MSC_VER
    return _byteswap_ulong(dword);
#endif
}

template <typename T>
bool ReadContent(std::fstream& file, T& structure)
{
    return static_cast<bool>(file.read(reinterpret_cast<char*>(&structure), sizeof(structure)));
}

std::filesystem::path CreateOutputDirectory(std::filesystem::path bnk_filename)
{
    const auto directory_name = bnk_filename.filename().replace_extension("");
    auto directory = bnk_filename.replace_filename(directory_name);
    create_directory(directory);
    return directory;
}

bool Compare(char* char_string, const std::string& string)
{
    return std::strncmp(char_string, string.c_str(), string.length()) == 0;
}

bool HasArgument(char* arguments[], const int argument_count, const std::string& argument)
{
    for (auto i = 0U; i < static_cast<std::size_t>(argument_count); ++i)
    {
        if (Compare(arguments[i], argument))
        {
            return true;
        }
    }

    return false;
}

std::vector<char> WemCopyChunk(std::vector<char> data, std::vector<char> wem_data, std::uint32_t& size)
{
    auto content = Section{}, content2 = Section{};
    std::uint32_t pos = 0xC, pos2 = 0xC;
    bool list = false, fmt = false;
    std::vector<char> ret_data;

    if (wem_data.size() < sizeof(Section)) {
        return wem_data;
    }

    std::copy_n(wem_data.data(), pos, std::back_inserter(ret_data));

    do
    {
        content = Section{};
        memcpy(&content, wem_data.data() + pos, sizeof(Section));
        if (Compare(content.sign, "fmt") ||
            Compare(content.sign, "cue") ||
            Compare(content.sign, "hash") ||
            Compare(content.sign, "smpl") ||
            Compare(content.sign, "vorb") ||
            Compare(content.sign, "LIST"))
        {
            std::copy_n(wem_data.data() + pos, content.size + sizeof(Section), std::back_inserter(ret_data));
            if (Compare(content.sign, "fmt")) {
                fmt = true;
            }
            else if (Compare(content.sign, "LIST"))
            {
                list = true;
            }
        }
        else {
            if (!list && fmt)
            {
                do
                {
                    content2 = Section{};
                    memcpy(&content2, data.data() + pos2, sizeof(Section));
                    if (Compare(content2.sign, "fmt") ||
                        Compare(content2.sign, "cue") ||
                        Compare(content.sign, "hash") ||
                        Compare(content2.sign, "smpl") ||
                        Compare(content2.sign, "vorb") ||
                        Compare(content2.sign, "LIST"))
                    {
                        if ((Compare(content2.sign, "LIST") && !list)) {
                            std::uint32_t fsize = 0;
                            memcpy(&fsize, wem_data.data() + 4, sizeof(std::uint32_t));
                            fsize += content2.size + sizeof(Section);
                            size += content2.size + sizeof(Section);
                            std::copy_n(data.data() + pos2, content2.size + sizeof(Section), std::back_inserter(ret_data));
                            memcpy(wem_data.data() + 4, &fsize, sizeof(std::uint32_t));
                        }
                    }
                    else {
                        break;
                    }
                    pos2 += content2.size + sizeof(Section);
                } while (true);
            }
            if (Compare(content.sign, "data")) {
                std::copy_n(wem_data.data() + pos, wem_data.size() - pos, std::back_inserter(ret_data));
            }
            pos += content.size + sizeof(Section);
            break;
        }
        pos += content.size + sizeof(Section);
    } while (true);

    return ret_data;
}

int main(int argument_count, char* arguments[])
{
    std::cout << "Wwise *.BNK File Extractor" << std::endl;
    std::cout << "(c) RAWR 2015-2022 & cleverzaq 2022-2023" << std::endl << std::endl;

    if (argument_count < 2)
    {
        std::cout << "Usage: bnkextr filename.bnk [/extract] [/import] [/swap] [/nodir] [/obj]" << std::endl;
        std::cout << "\t/extract - extract the files to the folder" << std::endl;
        std::cout << "\t/import - import the files from the folder" << std::endl;
        std::cout << "\t/swap - swap byte order (use it for unpacking 'Army of Two')" << std::endl;
        std::cout << "\t/nodir - create no additional directory for the *.wem files" << std::endl;
        std::cout << "\t/obj - generate an objects.txt file with the extracted object data" << std::endl;
        return EXIT_SUCCESS;
    }

    auto bnk_filename = std::filesystem::path{ std::string{ arguments[1] } };
    auto bnk_filename_out = std::filesystem::path{ std::string{ arguments[1] } + ".out" };
    auto swap_byte_order = HasArgument(arguments, argument_count, "/swap");
    auto no_directory = HasArgument(arguments, argument_count, "/nodir");
    auto dump_objects = HasArgument(arguments, argument_count, "/obj");
    auto extract = HasArgument(arguments, argument_count, "/extract");
    auto import = HasArgument(arguments, argument_count, "/import");

    auto bnk_file = std::fstream{ bnk_filename, std::ios::binary | std::ios::in };

    if (!bnk_file.is_open())
    {
        std::cout << "Can't open input file: " << bnk_filename << std::endl;
        return EXIT_FAILURE;
    }

    auto output_directory = bnk_filename.parent_path();

    if (!no_directory)
    {
        output_directory = CreateOutputDirectory(bnk_filename);
    }

    auto data_offset = std::size_t{ 0U }, hirc_offset = std::size_t{ 0U };
    auto files = std::vector<std::pair<Index, std::uint32_t>>{};
    auto content_section = Section{};
    auto content_index = Index{};
    auto bank_header = BankHeader{};

    auto objects = std::vector<std::pair<Object, std::uint32_t>>{};
    auto sound = std::map<std::uint32_t, Sound>{};
    auto event_objects = std::map<std::uint32_t, EventObject>{};
    auto event_action_objects = std::map<std::uint32_t, EventActionObject>{};

    auto rfiles = std::vector<std::pair<embeddedMedia, std::uint32_t>>{};
    auto riff = std::vector<std::pair<std::vector<char>, std::uint32_t>>{};
    auto riffnum = 0;

    while (ReadContent(bnk_file, content_section))
    {
        const std::size_t section_pos = bnk_file.tellg();

        if (swap_byte_order)
        {
            content_section.size = Swap32(content_section.size);
        }

        if (Compare(content_section.sign, "BKHD"))
        {
            ReadContent(bnk_file, bank_header);
            bnk_file.seekg(content_section.size - sizeof(BankHeader), std::ios_base::cur);

            std::cout << "Wwise Bank Version: " << bank_header.version << std::endl;
            std::cout << "Bank ID: " << bank_header.id << std::endl;
        }
        else if (Compare(content_section.sign, "AKPK"))
        {
            PCK_Header header = PCK_Header{};
            ReadContent(bnk_file, header);

            std::uint32_t stringDataCount = 0;
            ReadContent(bnk_file, stringDataCount);

            for (unsigned int i = 0; i < stringDataCount; i++)
            {
                stringData strdata = stringData{};
                ReadContent(bnk_file, strdata);
            }

            for (unsigned int i = 0; i < stringDataCount; i++)
            {
                char string = NULL;

                do
                {
                    ReadContent(bnk_file, string);
                    bnk_file.seekg(1, std::ios::cur);
                } while (string != NULL);
            }

            std::uint32_t bnkCount = 0, wemCount = 0;
            ReadContent(bnk_file, bnkCount);
            ReadContent(bnk_file, wemCount);

            for (unsigned int i = 0; i < wemCount; i++)
            {
                embeddedMedia media = embeddedMedia{};
                ReadContent(bnk_file, media);
                rfiles.push_back(std::make_pair(media, static_cast<std::uint32_t>(static_cast<std::uint32_t>(bnk_file.tellg()) - sizeof(embeddedMedia))));
            }
        }
        else if (Compare(content_section.sign, "DIDX"))
        {
            for (auto i = 0U; i < content_section.size; i += sizeof(content_index))
            {
                ReadContent(bnk_file, content_index);
                files.push_back(std::make_pair(content_index, static_cast<std::uint32_t>(static_cast<std::uint32_t>(bnk_file.tellg()) - sizeof(content_index))));
            }
        }
        else if (Compare(content_section.sign, "STID"))
        {

        }
        else if (Compare(content_section.sign, "DATA"))
        {
            data_offset = bnk_file.tellg();
        }
        else if (Compare(content_section.sign, "HIRC"))
        {
            hirc_offset = bnk_file.tellg();

            auto object_count = std::uint32_t{ 0 };
            ReadContent(bnk_file, object_count);

            for (auto i = 0U; i < object_count; ++i)
            {
                std::uint32_t header_position = static_cast<std::uint32_t>(bnk_file.tellg());
                auto object = Object{};
                ReadContent(bnk_file, object);

                if (object.type == ObjectType::Event)
                {
                    auto event = EventObject{};

                    if (bank_header.version >= 134)
                    {
                        auto count = std::uint8_t{ 0 };
                        ReadContent(bnk_file, count);
                        event.action_count = static_cast<std::uint32_t>(count);
                    }
                    else
                    {
                        ReadContent(bnk_file, event.action_count);
                    }

                    for (auto j = 0U; j < event.action_count; ++j)
                    {
                        auto action_id = std::uint32_t{ 0 };
                        ReadContent(bnk_file, action_id);
                        event.action_ids.push_back(action_id);
                    }

                    event_objects[object.id] = event;
                }
                else if (object.type == ObjectType::EventAction)
                {
                    auto event_action = EventActionObject{};

                    ReadContent(bnk_file, event_action.scope);
                    ReadContent(bnk_file, event_action.action_type);
                    ReadContent(bnk_file, event_action.game_object_id);
                    ReadContent(bnk_file, event_action.parameter_count);

                    for (auto j = 0U; j < static_cast<std::size_t>(event_action.parameter_count); ++j)
                    {
                        auto parameter_type = EventActionParameterType{};
                        ReadContent(bnk_file, parameter_type);
                        event_action.parameters_types.push_back(parameter_type);
                    }

                    for (auto j = 0U; j < static_cast<std::size_t>(event_action.parameter_count); ++j)
                    {
                        auto parameter = std::int8_t{ 0 };
                        ReadContent(bnk_file, parameter);
                        event_action.parameters.push_back(parameter);
                    }

                    event_action_objects[object.id] = event_action;
                }
                else if (object.type == ObjectType::Sound)
                {
                    auto sound_struct = Sound{};
                    ReadContent(bnk_file, sound_struct);
                    sound[object.id] = sound_struct;
                }

                bnk_file.seekg(header_position + object.size + sizeof(Object) - sizeof(std::uint32_t), std::ios_base::beg);
                objects.push_back(std::make_pair(object, header_position));
            }
        }
        else if (Compare(content_section.sign, "RIFF"))
        {
            auto wem_filename = output_directory;
            wem_filename = wem_filename.append(std::to_string(rfiles[riffnum].first.ID)).replace_extension(".wem");

            auto curr = bnk_file.tellg();
            bnk_file.seekg(std::uint32_t(curr) - sizeof(Section), std::ios_base::beg);
            auto data = std::vector<char>(content_section.size + sizeof(Section), 0U);
            bnk_file.read(data.data(), content_section.size + sizeof(Section));

            if (extract)
            {
                auto wem_file = std::fstream{ wem_filename, std::ios::binary | std::ios::out };
                std::cout << "Extracted: " << wem_filename << std::endl;
                wem_file.write(data.data(), content_section.size + sizeof(Section));
                wem_file.close();
            }
            if (import)
            {
                auto wem_file = std::fstream{ wem_filename, std::ios::binary | std::ios::in };
                if (!wem_file.is_open())
                {
                    std::cout << "Unable to open file " << wem_filename.string() << std::endl;
                    continue;
                }
                std::cout << "Imported: " << wem_filename << std::endl;

                wem_file.seekg(0, std::ios::end);
                std::uint32_t size = wem_file.tellg();
                wem_file.seekg(0, std::ios::beg);
                auto wem_data = std::vector<char>(size, 0U);
                wem_file.read(wem_data.data(), size);

                auto ret_data = WemCopyChunk(data, wem_data, size);
                rfiles[riffnum].first.Length = size;

                riff.push_back(std::pair(ret_data, static_cast<std::uint32_t>(static_cast<std::uint32_t>(curr) - sizeof(Section))));
                wem_file.close();
            }

            riffnum++;
        }

        bnk_file.seekg(section_pos + content_section.size);
    }

    bnk_file.clear();

    if (objects.size() > 0 && dump_objects)
    {
        auto object_filename = output_directory;
        object_filename = object_filename.append("objects.txt");
        auto object_file = std::fstream{ object_filename, std::ios::binary | std::ios::out };

        if (!object_file.is_open())
        {
            std::cout << "Unable to write objects file '" << object_filename.string() << std::endl;
            return EXIT_FAILURE;
        }

        for (auto vpos = 0U; vpos < objects.size(); vpos++)
        {
            object_file << "Object ID: " << objects[vpos].first.id << std::endl;
            object_file << "Type: " << GetObjectTypeName(objects[vpos].first.type) << std::endl;

            switch (objects[vpos].first.type)
            {
            case ObjectType::Event:
                object_file << "\tNumber of Actions: " << event_objects[objects[vpos].first.id].action_count << std::endl;

                for (auto& action_id : event_objects[objects[vpos].first.id].action_ids)
                {
                    object_file << "\tAction ID: " << action_id << std::endl;
                }
                break;

            case ObjectType::EventAction:
                object_file << "\tAction Scope: " << GetEventActionName(event_action_objects[objects[vpos].first.id].scope) << std::endl;
                object_file << "\tAction Type: " << GetActionName(event_action_objects[objects[vpos].first.id].action_type) << std::endl;
                object_file << "\tGame Object ID: " << static_cast<std::uint32_t>(event_action_objects[objects[vpos].first.id].game_object_id) << std::endl;
                object_file << "\tNumber of Parameters: " << static_cast<std::uint32_t>(event_action_objects[objects[vpos].first.id].parameter_count) << std::endl;

                for (auto j = 0; j < event_action_objects[objects[vpos].first.id].parameter_count; ++j)
                {
                    object_file << "\t\tParameter Type: " << GetEventActionParameterName(event_action_objects[objects[vpos].first.id].parameters_types[j]) << std::endl;
                    object_file << "\t\tParameter: " << static_cast<std::uint32_t>(event_action_objects[objects[vpos].first.id].parameters[j]) << std::endl;
                }
                break;

            case ObjectType::Sound:
                object_file << "\tAudioID: " << static_cast<std::uint32_t>(sound[objects[vpos].first.id].AudioID) << std::endl;
                object_file << "\tSourceID: " << static_cast<std::uint32_t>(sound[objects[vpos].first.id].SourceID) << std::endl;
                object_file << "\tSoundType: " << GetSoundTypeName(sound[objects[vpos].first.id].SoundType) << std::endl;
                object_file << "\tStorageType: " << GetStorageTypeName(sound[objects[vpos].first.id].state) << std::endl;
                break;
            }

            object_file << std::endl;
        }

        std::cout << "Objects file was written to: " << object_filename.string() << std::endl;
    }

    if (extract)
    {
        if (data_offset == 0U || files.empty())
        {
            return EXIT_SUCCESS;
        }

        std::cout << "Found " << files.size() << " WEM files" << std::endl;
        std::cout << "Start extracting..." << std::endl;

        for (auto vpos = 0U; vpos < files.size(); vpos++)
        {
            auto wem_filename = output_directory;
            wem_filename = wem_filename.append(std::to_string(files[vpos].first.id)).replace_extension(".wem");
            auto wem_file = std::fstream{ wem_filename, std::ios::out | std::ios::binary };
            std::cout << "Extracted: " << wem_filename << std::endl;

            if (swap_byte_order)
            {
                files[vpos].first.size = Swap32(files[vpos].first.size);
                files[vpos].first.offset = Swap32(files[vpos].first.offset);
            }

            if (!wem_file.is_open())
            {
                std::cout << "Unable to write file " << wem_filename.string() << std::endl;
                continue;
            }

            auto data = std::vector<char>(files[vpos].first.size, 0U);
            bnk_file.seekg(data_offset + files[vpos].first.offset);
            bnk_file.read(data.data(), files[vpos].first.size);
            wem_file.write(data.data(), files[vpos].first.size);
            wem_file.close();
        }

        std::cout << "Files were extracted to: " << output_directory.string() << std::endl;
    }
    else if (import) {
        if (data_offset != 0U || !files.empty())
        {
            int audio_size = 0, header_size = data_offset;
            for (unsigned int vpos = 0U, max = files.size(); vpos < max; vpos++)
            {
                audio_size += ((max - 1 == vpos) ? files[vpos].first.size : ALIGN(files[vpos].first.size, 16));
            }

            auto out_file = std::fstream{ bnk_filename_out, std::ios::binary | std::ios::out };
            auto out_data = std::vector<char>(header_size);
            bnk_file.seekg(0, std::ios::beg);
            bnk_file.read(out_data.data(), header_size);

            for (unsigned int vpos = 0U, max = files.size(); vpos < max; vpos++)
            {
                auto wem_filename = output_directory;
                wem_filename = wem_filename.append(std::to_string(files[vpos].first.id)).replace_extension(".wem");
                std::cout << "Imported: " << wem_filename << std::endl;
                auto wem_file = std::fstream{ wem_filename, std::ios::in | std::ios::binary };

                if (!wem_file.is_open())
                {
                    std::cout << "Unable to open file " << wem_filename.string() << std::endl;
                    continue;
                }

                wem_file.seekg(0, std::ios::end);
                std::uint32_t size = wem_file.tellg();
                wem_file.seekg(0, std::ios::beg);

                auto wem_data = std::vector<char>(size, 0U);
                wem_file.read(wem_data.data(), size);

                auto data = std::vector<char>(files[vpos].first.size, 0U);
                bnk_file.seekg(data_offset + files[vpos].first.offset);
                bnk_file.read(data.data(), files[vpos].first.size);

                auto ret_data = WemCopyChunk(data, wem_data, size);

                auto vsize = header_size + size, vsz = ((max - 1 == vpos) ? vsize : ALIGN(vsize, 16));
                out_data.resize(vsz);
                memcpy(out_data.data() + header_size, ret_data.data(), size);
                files[vpos].first.size = size;
                for (auto v = 0U; v < objects.size(); v++)
                {
                    if (sound[objects[v].first.id].AudioID == files[vpos].first.id)
                    {
                        sound[objects[v].first.id].InMemoryMediaSize = size;
                        break;
                    }
                }
                files[vpos].first.offset = header_size - data_offset;
                memcpy(&out_data[files[vpos].second], &files[vpos].first, sizeof(Index));

                header_size = vsz;
            }

            std::uint32_t data_size = header_size - data_offset;
            memcpy(&out_data[data_offset - 4], &data_size, sizeof(std::uint32_t));

            bnk_file.seekg(0, std::ios::end);
            std::int32_t size = bnk_file.tellg();
            bnk_file.seekg(audio_size + data_offset, std::ios::beg);
            size -= audio_size + data_offset;

            out_data.resize(header_size + size);
            bnk_file.read(out_data.data() + header_size, size);
            header_size += size;

            for (auto obj_pos = 0U; obj_pos < objects.size(); obj_pos++)
            {
                std::int32_t bnk_hirc_pos = header_size - size + objects[obj_pos].second - hirc_offset + sizeof(Object) - 1;
                switch (objects[obj_pos].first.type)
                {
                case ObjectType::Sound:
                    memcpy(out_data.data() + bnk_hirc_pos + sizeof(Object), &sound[objects[obj_pos].first.id], sizeof(Sound));
                    break;
                }
            }

            out_file.write(out_data.data(), header_size);
            out_file.close();
        }
        else if (riff.size() > 0 && rfiles.size() > 0) {
            auto out_file = std::fstream{ bnk_filename_out, std::ios::binary | std::ios::out };
            std::uint32_t fpos = riff[0].second;
            auto out_data = std::vector<char>(fpos);
            bnk_file.seekg(0, std::ios::beg);
            bnk_file.read(out_data.data(), fpos);

            for (auto vpos = 0U; vpos < rfiles.size(); vpos++)
            {
                rfiles[vpos].first.Offset = fpos;
                fpos += rfiles[vpos].first.Length;
                std::copy_n(riff[vpos].first.cbegin(), rfiles[vpos].first.Length, std::back_inserter(out_data));
                memcpy(&out_data[rfiles[vpos].second], &rfiles[vpos].first, sizeof(embeddedMedia));
            }

            out_file.write(out_data.data(), fpos);
            out_file.close();
        }
    }

    bnk_file.close();
}
