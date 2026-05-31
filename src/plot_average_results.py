import os
import sys
import pandas as pd
import matplotlib.pyplot as plt

def draw_chart(df, column_name, title, xlabel, output_path, is_percentage=False, reverse_color=False):
    """Universal chart drawing helper for averaged metrics"""
    plt.figure(figsize=(9, 4.5))
    
    # Color logic (Lower is better for AvgRank and DealIn)
    if reverse_color:
        colors = ['#4682B4' if val == df[column_name].min() else '#CD5C5C' for val in df[column_name]]
    else:
        colors = ['#4682B4' if val >= 0 else '#CD5C5C' for val in df[column_name]]
        if all(val >= 0 for val in df[column_name]):
            colors = ['#4682B4', '#5F9EA0', '#20B2AA', '#48D1CC'][:len(df)]

    bars = plt.barh(df['DisplayName'], df[column_name], color=colors, height=0.5)
    
    # Baseline at 0
    if df[column_name].min() < 0:
        plt.axvline(x=0, color='gray', linestyle='--', linewidth=0.8)

    plt.title(title, fontsize=13, fontweight='bold', pad=15)
    plt.xlabel(xlabel, fontsize=11)
    
    for bar in bars:
        width = bar.get_width()
        align = 'left' if width < 0 else 'right'
        offset = -5 if width < 0 else 5
        label = f'{width:+.2f}%' if is_percentage else f'{width:.2f}'
        if column_name == 'AvgScore' or (column_name == 'AvgPt' and df['Mode'].iloc[0] == 'round'):
            label = f'{int(width):,}'  # Format large scores with commas
            
        plt.annotate(label,
                     xy=(width, bar.get_y() + bar.get_height() / 2),
                     xytext=(offset, 0),
                     textcoords="offset points",
                     ha=align, va='center', fontsize=10, fontweight='bold')

    plt.grid(axis='x', linestyle=':', alpha=0.6)
    plt.gca().spines['top'].set_visible(False)
    plt.gca().spines['right'].set_visible(False)
    plt.tight_layout()
    
    img_dir = os.path.dirname(output_path)
    if img_dir and not os.path.exists(img_dir):
        os.makedirs(img_dir, exist_ok=True)

    plt.savefig(output_path, dpi=300)
    plt.close()
    print(f" -> Generated chart: {output_path}")

def parse_groups(args):
    """Parses grouping arguments like: --groups 2 0 1,2,3"""
    if '--groups' not in args:
        return None
    
    try:
        idx = args.index('--groups')
        num_groups = int(args[idx + 1])
        groups = []
        for i in range(num_groups):
            group_str = args[idx + 2 + i]
            group_members = [int(x) for x in group_str.split(',')]
            groups.append(group_members)
        return groups
    except Exception as e:
        print(f"Error parsing groups: {e}")
        return None

def main():
    usage = (
        "Usage:\n"
        "  python plot_average_results.py <csv1> [csv2 ...] [--groups <num> <g1> <g2> ...] <output_base_name>\n"
    )

    if len(sys.argv) < 3:
        print(usage)
        return

    base_name = sys.argv[-1]
    remaining_args = sys.argv[1:-1]

    groups = parse_groups(remaining_args)
    
    csv_paths = []
    if groups is not None:
        idx = remaining_args.index('--groups')
        csv_paths = remaining_args[:idx]
    else:
        csv_paths = remaining_args

    valid_dfs = []
    for path in csv_paths:
        if not os.path.exists(path):
            print(f"Error: File '{path}' does not exist. Aborting.")
            return
        valid_dfs.append(pd.read_csv(path))

    print(f"[Loading] Aggregating data from {len(valid_dfs)} CSV files...")

    combined_df = pd.concat(valid_dfs)
    num_cols = ['AvgScore', 'AvgPt', 'WinRate', 'DealIn', 'AvgTurn', 'AvgRank']
    
    # 這裡的 groupby 多加入了 'AIName'，用來動態保留 C++ 傳進來的真實 AI 名稱
    player_avg = combined_df.groupby(['Player', 'AIName'])[num_cols].mean().reset_index()
    player_avg['Mode'] = combined_df.groupby(['Player', 'AIName'])['Mode'].first().reset_index()['Mode']

    # 【核心改動】建立一個動態的 AI 對應字典，從 CSV 讀到什麼就用什麼！
    ai_mapping = {}
    for _, row in player_avg.iterrows():
        # 把讀進來的名字（如 "shanten"）轉成漂亮的字首大寫（"Shanten"）
        ai_mapping[int(row['Player'])] = str(row['AIName']).capitalize()
    
    final_rows = []
    mode = player_avg['Mode'].iloc[0]

    if groups:
        print(f"[Grouping] Applying customized group fusion: {groups}")
        for g_idx, g_members in enumerate(groups):
            sub_df = player_avg[player_avg['Player'].isin(g_members)]
            if sub_df.empty:
                continue
            
            mean_metrics = sub_df[num_cols].mean().to_dict()
            
            # 這裡的名字也是動態從映射字典撈取
            member_names = [ai_mapping.get(m, f"P{m}") for m in g_members]
            if len(member_names) == 1:
                display_name = f"Player {g_members[0]} ({member_names[0]})"
            else:
                display_name = f"Group ({', '.join(member_names)})"
                
            mean_metrics['DisplayName'] = display_name
            mean_metrics['Mode'] = mode
            final_rows.append(mean_metrics)
            
        plot_df = pd.DataFrame(final_rows)
    else:
        # 沒分組時，也完全動態讀取名稱
        player_avg['DisplayName'] = player_avg['Player'].map(
            lambda x: f"Player {x} ({ai_mapping.get(x, 'Unknown')})"
        )
        plot_df = player_avg

    print(f"[Plotting] Generating charts for {mode.upper()} mode...")

    if mode == 'match':
        metrics = [
            ('AvgScore', 'Averaged End Game Score', 'Points', False, False),
            ('AvgPt', 'Averaged Rank Points (Rank PT)', 'pt', False, False),
            ('WinRate', 'Averaged Win Rate (Heoul Rate)', 'Percentage', True, False),
            ('DealIn', 'Averaged Deal-in Rate', 'Percentage', True, True),
            ('AvgTurn', 'Averaged Winning Turn', 'Turns', False, True),
            ('AvgRank', 'Averaged Match Placement', 'Rank (1.0 ~ 4.0)', False, True)
        ]
    else:
        metrics = [
            ('AvgPt', 'Averaged Round Winning Points', 'Points', False, False),
            ('WinRate', 'Averaged Win Rate (Heoul Rate)', 'Percentage', True, False),
            ('DealIn', 'Averaged Deal-in Rate', 'Percentage', True, True),
            ('AvgTurn', 'Averaged Winning Turn', 'Turns', False, True)
        ]

    for col, title, xlabel, is_pct, rev_color in metrics:
        out_img_path = f"{base_name}_{col}.png"
        draw_chart(plot_df, col, title, xlabel, out_img_path, is_percentage=is_pct, reverse_color=rev_color)

    print("[Finished] Group-averaged charts generated successfully!")

if __name__ == '__main__':
    main()