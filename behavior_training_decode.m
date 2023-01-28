clear all

fpath = 'Z:\data\shulan\animal training\piston_twowhisker\#23228M';
phase = 3;
animal = '#23228M';

files = dir(fullfile(fpath,'*.csv'));
idx_f = [];

for i = 1:length(files)
    if contains(files(i).name, sprintf('phase%d', phase))
        idx_f = [idx_f, i];
    end
end

[~, I] = sortrows(struct2table(files(idx_f)), 'name');

files_phase = files(idx_f(I));

data = [];
data_count = 1;
j = 1;
for i = 1:length(files_phase)
    if i < j
        continue;
    end
    T = readtable(fullfile(fpath, files_phase(i).name));
    data(data_count).date = files_phase(i).name(end-22:end-13);
    data(data_count).phase = phase;
    data(data_count).dist = T.Var1;
    data(data_count).lick = T.Var2;
    data(data_count).valve = T.Var3;
    data(data_count).pistonGO = T.Var4;
    data(data_count).pistonNOGO = T.Var5;
    data(data_count).t = T.Var6;
    for j = i+1:length(files_phase)
        if ~strcmp(files_phase(j).name(end-22:end-13), data(data_count).date)
            break
        end
        T = readtable(fullfile(fpath, files_phase(j).name));
        data(data_count).dist = [data(data_count).dist; data(data_count).dist(end)+T.Var1];
        data(data_count).lick = [data(data_count).lick; T.Var2];
        data(data_count).valve = [data(data_count).valve; T.Var3];
        data(data_count).pistonGO = [data(data_count).pistonGO; T.Var4];
        data(data_count).pistonNOGO = [data(data_count).pistonNOGO; T.Var5];
        data(data_count).t = [data(data_count).t; data(data_count).t(end)+T.Var6];
    end
    data_count = data_count+1;
end

if phase==1   % lick for reward
    for i = 1:length(data)
        a = diff(data(i).t);
        lick_idx = find(data(i).lick);
        lick_dur = a(lick_idx+1);
        idx_remove = find(lick_dur>=0.5);
        lick_idx(idx_remove) = [];
        lick_dur(idx_remove) = [];
        data(i).lick_time = data(i).t(lick_idx);
        data(i).lick_rate = length(lick_idx)/(data(i).t(end)/60);  % lick/min
    end
elseif phase==2 % GO stimuli, reward given regardless
    for i = 1:length(data)
        detect_idx = find(data(i).lick & data(i).pistonGO);
        trial_num = length( find(data(i).pistonGO));
        data(i).detect_rate = length(detect_idx)/trial_num;
        data(i).detect_time = data(i).t(detect_idx);
        data(i).trial_time = data(i).t(find(data(i).pistonGO));
    end
elseif phase == 0 % GO stimuli, reward if lick is detected
    for i = 1:length(data)
        detect_idx = find(data(i).lick & data(i).pistonGO);
        trial_num = length( find(data(i).pistonGO));
        data(i).detect_rate = length(detect_idx)/trial_num;
        data(i).detect_time = data(i).t(detect_idx);
        data(i).trial_time = data(i).t(find(data(i).pistonGO));
    end
elseif phase == 3 % GO & NOGO stimuli, reword at hit
    for i = [1:length(data)]
        hit_idx = find(data(i).lick & data(i).pistonGO);
        fa_idx = find(data(i).lick & data(i).pistonNOGO);
        GO_trial_num = length( find(data(i).pistonGO));
        NOGO_trial_num = length( find(data(i).pistonNOGO));
        data(i).hit_rate = length(hit_idx)/GO_trial_num;
        data(i).hit_time = data(i).t(hit_idx);
        data(i).GO_trial_time = data(i).t( find(data(i).pistonGO));
        data(i).NOGO_trial_time = data(i).t( find(data(i).pistonNOGO));
        data(i).fa_rate = length(fa_idx)/NOGO_trial_num;
        data(i).fa_time = data(i).t(fa_idx);
        edges =  0:5*60:data(i).t(end);
        hit_hist = histcounts(data(i).hit_time, edges);
        GO_hist = histcounts(data(i).GO_trial_time, edges);
        fa_hist = histcounts(data(i).fa_time, edges);
        NOGO_hist = histcounts(data(i).NOGO_trial_time, edges);
        data(i).ROC = [fa_hist./NOGO_hist; hit_hist./GO_hist];
        data(i).ROC(isnan(data(i).ROC))=0;
        data(i).ROC = data(i).ROC';
        data(i).ROC = sortrows(data(i).ROC,1);
    end
end

save(fullfile(fpath, sprintf('%s_phase%d.mat', animal, phase)), 'data')